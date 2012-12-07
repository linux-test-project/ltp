  /*
     @pt:XSI
     Test that the function:
     void (*bsd_signal(int, void (*)(int)))(int);
     is declared.
   */

#include <signal.h>

typedef void (*(*bsd_signal_test) (int, void (*)(int))) (int);

int main(void)
{
	bsd_signal_test dummyvar;
	dummyvar = bsd_signal;
	return 0;
}
