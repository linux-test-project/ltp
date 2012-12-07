  /*
     Test that the function:
     int sigwait(const sigset_t *restrict, int *restrict);
     is declared.
   */

#include <signal.h>

typedef int (*sigwait_test) (const sigset_t * restrict, int *restrict);

int dummyfcn(void)
{
	sigwait_test dummyvar;
	dummyvar = sigwait;
	return 0;
}
