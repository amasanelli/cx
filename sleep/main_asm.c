#define SYS_WRITE 1
#define SYS_NANOSLEEP 35

typedef struct timespec
{
  long tv_sec;
  long tv_nsec;
} timespec;

/* implemented in syscall.s */
long syscall1(long number, long arg1);
long syscall2(long number, long arg1, long arg2);
long syscall3(long number, long arg1, long arg2, long arg3);

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
  long unsigned result = 0;

  while (*cursor)
  {
    cursor++;
  }

  result = cursor - string;
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
  char *raw_seconds = 0;
  long seconds = 0;

  if (argc != 2)
  {
    print("Usage: sleep NUMBER\nPause for NUMBER seconds\n");
    return 1;
  }

  raw_seconds = argv[1];
  seconds = parse_int(raw_seconds);

  print("Sleeping for ");
  print(raw_seconds);
  print(" seconds\n");
  sleep(seconds);

  return 0;
}
