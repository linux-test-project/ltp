  /*
     Test that the function:
     int killpg(pid_t, int);
     is declared.
   */

#include <signal.h>
#include <sys/types.h>

typedef int (*killpg_test) (pid_t, int);

int dummyfcn(void)
{
	killpg_test dummyvar;
	dummyvar = killpg;
	return 0;
}
