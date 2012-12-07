  /*
     Test that the function:
     int sigdelset(sigset_t *, int);
     is declared.
   */

#include <signal.h>

typedef int (*sigdelset_test) (sigset_t *, int);

int dummyfcn(void)
{
	sigdelset_test dummyvar;
	dummyvar = sigdelset;
	return 0;
}
