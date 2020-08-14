  /*
     Test that the function:
     int sigrelse(int);
     is declared.
   */

#include <signal.h>

typedef int (*sigrelse_test) (int);

static int dummyfcn(void)
{
	sigrelse_test dummyvar;
	dummyvar = sigrelse;
	return 0;
}
