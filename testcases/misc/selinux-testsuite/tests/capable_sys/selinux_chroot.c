#include<stdio.h>

/*
 * Test the chroot() call on a directory whose name is given as the first 
 * argument. This call will result in a CAP_SYS_CHROOT capable check.
 */
int main(int argc, char **argv) {

  int rc;

  if( argc != 2 ) {
    printf("usage: %s pathname\n", argv[0]);
    exit(2);
  }

  rc = chroot(argv[1]);
  if( rc != 0 ) {
    perror("test_chroot:chroot");
    exit(1);
  }

  exit(0);

}
