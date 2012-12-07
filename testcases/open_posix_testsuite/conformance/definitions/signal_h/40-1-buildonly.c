  /*
     @:pt:XSI
     Test that the function:
     int sigpause(int);
     is declared.
   */

#include <signal.h>

typedef int (*sigpause_test) (int);

int dummyfcn(void)
{
	sigpause_test dummyvar;
	dummyvar = sigpause;
	return 0;
}
