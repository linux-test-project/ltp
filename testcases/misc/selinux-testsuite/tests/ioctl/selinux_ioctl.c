#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<linux/fs.h>
#include<linux/ext2_fs.h>

/*
 * Test the ioctl() calls on a file whose name is given as the first 
 * argument. This program expects the domain it is running under to have
 * wide access to the given file.
 */
int main(int argc, char **argv) {

  int fd;
  int rc;
  int val = 0;

  fd = open(argv[1], O_RDONLY, 0);
  
  if(fd == -1) {
    perror("test_ioctl:open");
    exit(1);
  }

  /* This one should hit the FILE__GETATTR test */
  rc = ioctl(fd, FIGETBSZ, &val);
  if( rc != 0 ) {
    perror("test_ioctl:FIGETBSZ");
    exit(1);
  }

  /* This one should hit the FILE__IOCTL test */
  rc = ioctl(fd, FIOCLEX);
  if( rc != 0 ) {
    perror("test_ioctl:FIOCLEX");
    exit(1);
  }

  /* This one should hit the normal file descriptor use test */
  rc = ioctl(fd, FIONBIO, &val);
  if( rc != 0 ) {
    perror("test_ioctl:FIONBIO");
    exit(1);
  }

  val = 0;
  /* This one should hit the FILE__SETATTR test */
  rc = ioctl(fd, EXT2_IOC_SETVERSION, &val);
  if( rc != 0 ) {
    perror("test_ioctl:EXT2_IOC_SETVERSION");
    exit(1);
  }

  close(fd);
  exit(0);

}
