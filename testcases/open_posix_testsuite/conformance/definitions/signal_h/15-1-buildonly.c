  /*
     Test the definition of sigaction.
   */

#include <signal.h>

struct sigaction this_type_should_exist, t;
extern void signal_handler(int);
sigset_t *set;
int flags;
extern void signal_action(int, siginfo_t *, void *);

int dummyfcn(void)
{
	t.sa_handler = signal_handler;
	set = &t.sa_mask;
	flags = t.sa_flags;
	t.sa_sigaction = signal_action;

	return 0;
}
