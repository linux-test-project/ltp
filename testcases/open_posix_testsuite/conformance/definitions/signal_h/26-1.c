  /*
  Test that the function:
   int pthread_kill(pthread_t, int);
  is declared.
  */

#include <signal.h>

typedef int (*pthread_kill_test)(pthread_t, int);

int dummyfcn (void)
{
	pthread_kill_test dummyvar;
	dummyvar = pthread_kill;
	return 0;
}
