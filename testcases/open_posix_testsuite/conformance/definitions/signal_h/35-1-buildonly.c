  /*
     @:pt:XSI
     Test that the function:
     int sighold(int);
     is declared.
   */

#include <signal.h>

typedef int (*sighold_test) (int);

int dummyfcn(void)
{
	sighold_test dummyvar;
	dummyvar = sighold;
	return 0;
}
