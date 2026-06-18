/* use intel syntax: "mov rax, rdi" instead of AT&T "movq %rdi, %rax" (reversed operands, % prefixes) */
.intel_syntax noprefix  /* noprefix: omit register prefixes (rax not %rax) */
.text                   /* marks start of executable code section (vs .data for variables) */

/* syscall1(number, arg1)
   System V ABI passes: number=rdi, arg1=rsi
   Linux syscall expects: number=rax, arg1=rdi */
.global syscall1
syscall1:
    mov rax, rdi        /* syscall number */
    mov rdi, rsi        /* arg1 */
    syscall
    ret

/* syscall2(number, arg1, arg2)
   System V ABI passes: number=rdi, arg1=rsi, arg2=rdx
   Linux syscall expects: number=rax, arg1=rdi, arg2=rsi */
.global syscall2
syscall2:
    mov rax, rdi        /* syscall number */
    mov rdi, rsi        /* arg1 */
    mov rsi, rdx        /* arg2 */
    syscall
    ret

/* syscall3(number, arg1, arg2, arg3)
   System V ABI passes: number=rdi, arg1=rsi, arg2=rdx, arg3=rcx
   Linux syscall expects: number=rax, arg1=rdi, arg2=rsi, arg3=rdx */
.global syscall3
syscall3:
    mov rax, rdi        /* syscall number */
    mov rdi, rsi        /* arg1 */
    mov rsi, rdx        /* arg2 */
    mov rdx, rcx        /* arg3 */
    syscall
    ret

/* _start: process entry point, no prologue/epilogue, no caller, no return address.
   kernel places argc at top of stack, argv[] immediately after. */
.global _start
_start:
    xor ebp, ebp        /* zero rbp -- marks outermost frame for stack unwinders */
    mov rdi, [rsp]      /* argc -- at top of stack on entry */
    lea rsi, [rsp+8]    /* argv -- right after argc */
    and rsp, -16        /* align stack to 16 bytes -- System V ABI requires before call */
    call main
    mov rdi, rax        /* exit code from main return value */
    call exit

/* exit: call SYS_EXIT syscall directly, never returns */
.global exit
exit:
    mov rax, 60         /* SYS_EXIT */
    syscall
    ud2                 /* trap if syscall somehow returns -- should never execute */
