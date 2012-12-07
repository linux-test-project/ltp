  /*
     Test that the function:
     int sigfillset(sigset_t *);
     is declared.
   */

#include <signal.h>

typedef int (*sigfillset_test) (sigset_t *);

int dummyfcn(void)
{
	sigfillset_test dummyvar;
	dummyvar = sigfillset;
	return 0;
}
