#include<stdio.h>

/*
 * Test the sethostname() call.
 * This call will result in a CAP_SYS_ADMIN capable check.
 */
int main(int argc, char **argv) {

  int rc;
  char buf[255];

  rc = gethostname(buf, sizeof(buf));
  if( rc != 0 ) {
    perror("test_sethostname:gethostname");
    exit(2);
  }

  rc = sethostname(buf, strlen(buf));
  if( rc != 0 ) {
    perror("test_sethostname:sethostname");
    exit(1);
  }

  exit(0);

}
