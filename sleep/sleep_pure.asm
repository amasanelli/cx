.intel_syntax noprefix
.text

.global _start
_start:
    mov rax, [rsp]      /* argc */
    cmp rax, 2
    jne .usage

    mov rsi, [rsp+16]   /* argv[1] -- rsp+8=argv[0], rsp+16=argv[1] */
    mov r8, rsi         /* save start of argv[1] to print it and compute length later */

    /* parse decimal integer from string into rbx */
    xor rbx, rbx
.parse:
    movzx rax, byte ptr [rsi]
    test rax, rax       /* null terminator = done */
    jz .sleep
    sub rax, '0'
    cmp rax, 9
    ja .usage           /* non-digit = err */
    imul rbx, rbx, 10
    add rbx, rax
    inc rsi
    jmp .parse

.sleep:
    /* rsi = end of digits, r8 = start -- compute length without int-to-string conversion */
    mov r9, rsi
    sub r9, r8          /* r9 = length of number string */

    /* write "Sleeping for " */
    mov rax, 1
    mov rdi, 1
    lea rsi, [rip + sleeping_msg]
    mov rdx, sleeping_msg_end - sleeping_msg
    syscall

    /* write the number (reuse argv[1] string directly) */
    mov rax, 1
    mov rdi, 1
    mov rsi, r8
    mov rdx, r9
    syscall

    /* write " seconds\n" */
    mov rax, 1
    mov rdi, 1
    lea rsi, [rip + seconds_msg]
    mov rdx, seconds_msg_end - seconds_msg
    syscall

    /* build timespec { tv_sec, tv_nsec } on stack */
    sub rsp, 16
    mov [rsp], rbx             /* tv_sec = parsed seconds */
    mov qword ptr [rsp+8], 0   /* tv_nsec = 0 */

    /* nanosleep(&timespec, NULL) */
    mov rax, 35
    mov rdi, rsp
    xor rsi, rsi
    syscall

    /* exit(0) */
    mov rax, 60
    xor rdi, rdi
    syscall
    ud2

.usage:
    /* write(stdout, msg, len) */
    mov rax, 1
    mov rdi, 1
    lea rsi, [rip + usage_msg]
    mov rdx, usage_msg_end - usage_msg  /* constant expression -- GAS emits immediate, not memory load */
    syscall

    /* exit(1) */
    mov rax, 60
    mov rdi, 1
    syscall
    ud2

.section .rodata
sleeping_msg:
    .ascii "Sleeping for "
sleeping_msg_end:
seconds_msg:
    .ascii " seconds\n"
seconds_msg_end:
usage_msg:
    .ascii "Usage: sleep NUMBER\nPause for NUMBER seconds\n"
usage_msg_end:
