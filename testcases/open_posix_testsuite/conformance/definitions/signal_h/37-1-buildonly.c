  /*
     Test that the function:
     int siginterrupt(int, int);
     is declared.
   */

#include <signal.h>

typedef int (*siginterrupt_test) (int, int);

static int dummyfcn(void)
{
	siginterrupt_test dummyvar;
	dummyvar = siginterrupt;
	return 0;
}
