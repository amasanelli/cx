#include "functions.h"

int main(void)
{
  int skt = -1;

  if (open_raw_eth_socket(&skt) != OK)
  {
    perror("open_raw_eth_socket");
    return 1;
  }

  printf("listening on all interfaces\n\n");

  for (;;)
  {
    if (pong(skt) != OK)
    {
      perror("pong");
      break;
    }
  }

  close(skt);
  return 0;
}
