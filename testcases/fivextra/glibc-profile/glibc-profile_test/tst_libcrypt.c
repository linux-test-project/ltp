#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>

int
test_md5c ( void )
{
  const char salt[] = "$1$saltstring";
  char *cp;

  cp = crypt ("Hello world!", salt);
  if (strcmp ("$1$saltstri$YMyguxXMBpd2TEZ.vS/3q1", cp)) { 
      fprintf(stderr, "Failed md5 crypt test!\n");
      return EXIT_FAILURE;
  }
  fprintf(stdout, "Passed md5 crypt test!\n");
  return EXIT_SUCCESS;
}

int main (void)
 {
    int ret;  

     ret = test_md5c ();

      return ret; 
  }
