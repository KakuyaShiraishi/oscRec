#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "oscRec.h"

static volatile bool keepRunning = true;

static void sigintHandler(int x) {
  keepRunning = false;
}

int main(int argc, char *argv[]) {

  char buffer[2048];

  printf("Starting write tests:\n");
  int len = 0;
  char blob[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
  len = tosc_write(buffer, sizeof(buffer), "/address", "fsibTFNI",
      1.0f, "hello world", -1, sizeof(blob), blob);
  tosc_printOscBuffer(buffer, len);
  printf("done.\n");

  signal(SIGINT, &sigintHandler);

  const int fd = socket(AF_INET, SOCK_DGRAM, 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(9000);
  sin.sin_addr.s_addr = INADDR_ANY;
  bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  printf("tinyosc is now listening on port 9000.\n");
  printf("Press Ctrl+C to stop.\n");

  while (keepRunning) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(fd, &readSet);
    struct timeval timeout = {1, 0};
    if (select(fd+1, &readSet, NULL, NULL, &timeout) > 0) {
      struct sockaddr sa;
      socklen_t sa_len = sizeof(struct sockaddr_in);
      int len = 0;
      while ((len = (int) recvfrom(fd, buffer, sizeof(buffer), 0, &sa, &sa_len)) > 0) {
        tosc_printOscBuffer(buffer, len);
      }
    }
  }

  close(fd);

  return 0;
}
