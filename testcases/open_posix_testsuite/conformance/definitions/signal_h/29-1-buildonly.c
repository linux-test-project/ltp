  /*
     Test that the function:
     int sigaction(int, const struct sigaction *restrict,
     struct sigaction *restrict);
     is declared.
     Removed restrict keyword from typedef.
   */

#include <signal.h>

typedef int (*sigaction_test) (int, const struct sigaction *,
			       struct sigaction *);

int dummyfcn(void)
{
	sigaction_test dummyvar;
	dummyvar = sigaction;
	return 0;
}
