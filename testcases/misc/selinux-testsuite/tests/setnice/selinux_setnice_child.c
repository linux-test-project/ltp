#include <unistd.h>

int main(void) 
{
  char buf[1];
  int rc;

  buf[0] = 0;
  rc = write(1, buf, sizeof buf);
  if (rc < 0) {
    perror("write");
    exit(-1);
  }
  rc = read(0, buf, sizeof buf);
  if (rc < 0) {
    perror("read");
    exit(-1);
  }  
  exit(0);
}

