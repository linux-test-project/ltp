  /*
     Test the definition of sigstack.
   */

#include <signal.h>

struct sigstack this_type_should_exist, t;
int onstack;
void *sp;

int dummyfcn(void)
{
	sp = t.ss_sp;
	onstack = t.ss_onstack;

	return 0;
}
