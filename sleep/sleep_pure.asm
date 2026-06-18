.intel_syntax noprefix
.text

.global _start
_start:
    mov rax, [rsp]      /* argc */
    cmp rax, 2
    jne .usage

    mov rsi, [rsp+16]   /* argv[1] -- rsp+8=argv[0], rsp+16=argv[1] */

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
usage_msg:
    .ascii "Usage: sleep NUMBER\nPause for NUMBER seconds\n"
usage_msg_end:
