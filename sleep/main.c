#define SYS_WRITE 1
#define SYS_NANOSLEEP 35
#define SYS_EXIT 60

typedef struct timespec
{
  long tv_sec;
  long tv_nsec;
} timespec;

long syscall1(long number, long arg1)
{
  long result;

  /* volatile: no reorder/deduplicate/eliminate. "=a"=rax out, "a"/"D"=rdi/rsi in.
     rcx/r11 clobbered: syscall saves rip->rcx and rflags->r11 so sysretq can restore
     them on return; compiler cannot infer this -- it only sees opaque "syscall" bytes.
     without declaring clobbers, compiler may keep live values in rcx/r11 across the
     call and read garbage after. memory barrier: compiler may cache globals in regs or
     reorder stores for optimization -- barrier forces all pending reads/writes to
     complete before the syscall and prevents moving any after it. */
  __asm__ __volatile__("syscall"
                       : "=a"(result)
                       : "a"(number), "D"(arg1)
                       : "rcx", "r11", "memory");

  return result;
}

long syscall2(long number, long arg1, long arg2)
{
  long result;

  /* volatile: no reorder/deduplicate/eliminate. "=a"=rax out, "a"/"D"/"S"=rdi/rsi/rdx in.
     rcx/r11 clobbered: cpu saves rip->rcx and rflags->r11 on syscall entry, destroying them.
     memory = compiler barrier, prevents reordering loads/stores around syscall. */
  __asm__ __volatile__("syscall"
                       : "=a"(result)
                       : "a"(number), "D"(arg1), "S"(arg2)
                       : "rcx", "r11", "memory");

  return result;
}

long syscall3(long number, long arg1, long arg2, long arg3)
{
  long result;

  /* volatile: no reorder/deduplicate/eliminate. "=a"=rax out, "a"/"D"/"S"/"d"=rdi/rsi/rdx/rcx in.
     rcx/r11 clobbered: cpu saves rip->rcx and rflags->r11 on syscall entry, destroying them.
     memory = compiler barrier, prevents reordering loads/stores around syscall. */
  __asm__ __volatile__("syscall"
                       : "=a"(result)
                       : "a"(number), "D"(arg1), "S"(arg2), "d"(arg3)
                       : "rcx", "r11", "memory");

  return result;
}

int parse_int(const char *raw_int)
{
  int result = 0;
  const char *cursor = raw_int;

  while (*cursor >= '0' && *cursor <= '9')
  {
    result = result * 10 + (*cursor - '0');
    cursor++;
  }

  return result;
}

long unsigned strlen(const char *string)
{
  const char *cursor = string;
  while (*cursor)
  {
    cursor++;
  }

  long unsigned result = cursor - string;
  return result;
}

void print(const char *string)
{
  syscall3(SYS_WRITE, 1, (long)string, strlen(string));
}

void sleep(long seconds)
{
  timespec duration = {0};
  duration.tv_sec = seconds;

  syscall2(SYS_NANOSLEEP, (long)(&duration), 0);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    print("Usage: sleep NUMBER\nPause for NUMBER seconds\n");
    return 1;
  }

  char *raw_seconds = argv[1];
  long seconds = parse_int(raw_seconds);

  print("Sleeping for ");
  print(raw_seconds);
  print(" seconds\n");
  sleep(seconds);

  return 0;
}

/* without noreturn, compiler emits epilogue after syscall and may optimize
   out code paths incorrectly -- callers also lose optimization opportunities */
__attribute__((noreturn)) void exit(int code)
{
  syscall1(SYS_EXIT, code);
  /* prevent compiler fallthrough -- syscall never returns but compiler doesn't know */
  for (;;)
  {
  }
}

/* naked: emit no prologue/epilogue. _start is the process entry point -- no caller,
   no return address on stack, no previous frame. compiler-generated prologue would
   corrupt rsp before argc/argv can be read from it. only the inline asm below runs. */
__attribute__((naked)) void _start()
{
  __asm__ __volatile__(
      "xor %ebp, %ebp\n"    /* zero rbp -- ABI marks outermost frame for stack unwinders */
      "mov (%rsp), %rdi\n"  /* argc -- kernel places it at top of stack on process entry */
      "lea 8(%rsp), %rsi\n" /* argv -- array starts right after argc on stack */
      "and $-16, %rsp\n"    /* align stack to 16 bytes -- System V ABI requires before call */
      "call main\n"         /* main(argc, argv) -- rdi/rsi already set above */
      "mov %rax, %rdi\n"    /* move main return value (exit code) to rdi -- first arg */
      "call exit\n"         /* exit(code) -- never returns */
  );
}
