  /*
     Test that the function:
     int sigemptyset(sigset_t *);
     is declared.
   */

#include <signal.h>

typedef int (*sigemptyset_test) (sigset_t *);

int dummyfcn(void)
{
	sigemptyset_test dummyvar;
	dummyvar = sigemptyset;
	return 0;
}
