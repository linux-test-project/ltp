#include<stdio.h>
#include<errno.h>

/*
 * Test the nice() system call.
 * This call will result in a CAP_SYS_NICE capable check.
 */
int main(int argc, char **argv) {

  int rc;

  rc = nice(-10);
  if( rc == -1) {
    perror("test_nice:nice");
    exit(1);
  }

  exit(0);

}
