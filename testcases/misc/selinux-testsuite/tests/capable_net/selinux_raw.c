#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>

/*
 * Test the bind() operation for a raw socket.
 */
int main(int argc, char **argv) {

  int fd;

  fd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
  if(fd == -1) {
    perror("test_raw:socket");
    exit(1);
  }

  close(fd);
  exit(0);

}
