  /*
     @:pt:XSI
     Test that the function:
     int sigignore(int);
     is declared.
   */

#include <signal.h>

typedef int (*sigignore_test) (int);

static int dummyfcn(void)
{
	sigignore_test dummyvar;
	dummyvar = sigignore;
	return 0;
}
