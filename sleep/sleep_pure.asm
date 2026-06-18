.intel_syntax noprefix                          /* intel syntax: "mov dst, src" not AT&T "mov src, dst" */
.text                                           /* executable code section */

.global _start
_start:
    mov rax, [rsp]                              /* load argc from top of stack (kernel places it there) */
    cmp rax, 2                                  /* expect exactly one argument */
    jne .usage                                  /* wrong arg count -- show usage */

    mov rsi, [rsp+16]                           /* argv[1] -- rsp+8=argv[0], rsp+16=argv[1] */
    mov r8, rsi                                 /* save argv[1] start to reuse string and compute length */

    xor rbx, rbx                                /* rbx = 0, accumulates parsed integer */
.parse:
    movzx rax, byte ptr [rsi]                   /* load next character, zero-extend to 64 bits */
    test rax, rax                               /* check for null terminator */
    jz .sleep                                   /* end of string -- done parsing */
    sub rax, '0'                                /* convert ascii digit to numeric value */
    cmp rax, 9                                  /* check if character was a valid digit */
    ja .usage                                   /* value > 9 means non-digit -- show usage */
    imul rbx, rbx, 10                           /* shift accumulated value left one decimal place */
    add rbx, rax                                /* add new digit */
    inc rsi                                     /* advance to next character */
    jmp .parse                                  /* loop */

.sleep:
    mov r9, rsi                                 /* r9 = end of digit string */
    sub r9, r8                                  /* r9 = length of argv[1] (end - start) */

    mov rax, 1                                  /* sys_write */
    mov rdi, 1                                  /* fd = stdout */
    lea rsi, [rip + sleeping_msg]               /* rip-relative address of "Sleeping for " */
    mov rdx, sleeping_msg_end - sleeping_msg    /* length as compile-time constant */
    syscall                                     /* write("Sleeping for ") */

    mov rax, 1                                  /* sys_write */
    mov rdi, 1                                  /* fd = stdout */
    mov rsi, r8                                 /* argv[1] start -- reuse original string, no int-to-string needed */
    mov rdx, r9                                 /* argv[1] length computed above */
    syscall                                     /* write(argv[1]) */

    mov rax, 1                                  /* sys_write */
    mov rdi, 1                                  /* fd = stdout */
    lea rsi, [rip + seconds_msg]                /* rip-relative address of " seconds\n" */
    mov rdx, seconds_msg_end - seconds_msg      /* length as compile-time constant */
    syscall                                     /* write(" seconds\n") */

    sub rsp, 16                                 /* allocate 16 bytes on stack for timespec struct */
    mov [rsp], rbx                              /* timespec.tv_sec = parsed seconds */
    mov qword ptr [rsp+8], 0                    /* timespec.tv_nsec = 0 */

    mov rax, 35                                 /* sys_nanosleep */
    mov rdi, rsp                                /* arg1 = &timespec */
    xor rsi, rsi                                /* arg2 = NULL (don't capture remaining time) */
    syscall                                     /* nanosleep(&timespec, NULL) */

    mov rax, 60                                 /* sys_exit */
    xor rdi, rdi                                /* exit code = 0 */
    syscall                                     /* exit(0) */
    ud2                                         /* trap -- should never reach here */

.usage:
    mov rax, 1                                  /* sys_write */
    mov rdi, 1                                  /* fd = stdout */
    lea rsi, [rip + usage_msg]                  /* rip-relative address of usage string */
    mov rdx, usage_msg_end - usage_msg          /* length as compile-time constant */
    syscall                                     /* write(usage_msg) */

    mov rax, 60                                 /* sys_exit */
    mov rdi, 1                                  /* exit code = 1 (error) */
    syscall                                     /* exit(1) */
    ud2                                         /* trap -- should never reach here */

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
