#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>

/*
 * Test the bind() operation for a socket that is protected (< 1024).
 */
int main(int argc, char **argv) {

  int fd;
  int aport;

  aport = IPPORT_RESERVED - 1;
  fd = rresvport(&aport);
  if(fd == -1) {
    perror("test_bind:rresvport");
    exit(1);
  }

  close(fd);
  exit(0);

}
