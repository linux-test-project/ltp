  /*
     Test that the function:
     int sigqueue(pid_t, int, const union sigval);
     is declared.
   */

#include <signal.h>
#include <sys/types.h>

typedef int (*sigqueue_test) (pid_t, int, const union sigval);

int dummyfcn(void)
{
	sigqueue_test dummyvar;
	dummyvar = sigqueue;
	return 0;
}
