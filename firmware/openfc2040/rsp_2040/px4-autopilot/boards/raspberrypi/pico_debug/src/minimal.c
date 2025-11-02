#include <nuttx/config.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  printf("USB CONSOLE TEST\n");
  for (int i = 0; ; i++) {
    printf("[%d] Hello from RP2040!\n", i);
    usleep(500000);
  }
  return 0;
}
