  /*
     Test that the function:
     int raise(int);
     is declared.
   */

#include <signal.h>

typedef int (*raise_test) (int);

int dummyfcn(void)
{
	raise_test dummyvar;
	dummyvar = raise;
	return 0;
}
