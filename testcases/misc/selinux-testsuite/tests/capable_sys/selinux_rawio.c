#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/fs.h>

/*
 * Test the FIBMAP ioctl() call on a file whose name is given as the first 
 * argument. This ioctl will result in a CAP_SYS_RAWIO capable check.
 */
int main(int argc, char **argv) {

  int fd;
  int rc;
  int val = 0;

  if( argc != 2 ) {
    printf("usage: %s filename\n", argv[0]);
    exit(2);
  }

  fd = open(argv[1], O_RDONLY, 0);
  
  if(fd == -1) {
    perror("test_rawio:open");
    exit(2);
  }

  val = 0;
  rc = ioctl(fd, FIBMAP, &val);
  if( rc != 0 ) {
    perror("test_rawio:FIBMAP");
    exit(1);
  }

  close(fd);
  exit(0);

}
