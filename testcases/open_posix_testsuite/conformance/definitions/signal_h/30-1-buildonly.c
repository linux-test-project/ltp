  /*
     Test that the function:
     int sigaddset(sigset_t *, int);
     is declared.
   */

#include <signal.h>

typedef int (*sigaddset_test) (sigset_t *, int);

int dummyfcn(void)
{
	sigaddset_test dummyvar;
	dummyvar = sigaddset;
	return 0;
}
