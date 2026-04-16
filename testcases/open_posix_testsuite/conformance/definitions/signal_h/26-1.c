  /*
     Test that the function:
     int pthread_kill(pthread_t, int);
     is declared.
   */

#include <pthread.h>
#include <signal.h>
#include "posixtest.h"

typedef int (*pthread_kill_test) (pthread_t, int);

int test_main(int argc PTS_ATTRIBUTE_UNUSED, char **argv PTS_ATTRIBUTE_UNUSED)
{
	pthread_kill_test dummyvar;
	dummyvar = pthread_kill;
	return 0;
}
