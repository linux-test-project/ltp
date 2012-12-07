  /*
     Test that the function:
     int sigismember(const sigset_t *, int);
     is declared.
   */

#include <signal.h>

typedef int (*sigismember_test) (const sigset_t *, int);

int dummyfcn(void)
{
	sigismember_test dummyvar;
	dummyvar = sigismember;
	return 0;
}
