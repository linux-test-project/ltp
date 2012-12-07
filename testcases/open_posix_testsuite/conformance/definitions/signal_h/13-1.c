  /*
     Test that 0 is reserved for the null signal.
     This test must be executed.
   */

#include <signal.h>
#include <stdio.h>

int main()
{
	if ((0 == SIGABRT) ||
	    (0 == SIGALRM) ||
	    (0 == SIGBUS) ||
	    (0 == SIGCHLD) ||
	    (0 == SIGCONT) ||
	    (0 == SIGFPE) ||
	    (0 == SIGHUP) ||
	    (0 == SIGILL) ||
	    (0 == SIGINT) ||
	    (0 == SIGKILL) ||
	    (0 == SIGPIPE) ||
	    (0 == SIGQUIT) ||
	    (0 == SIGSEGV) ||
	    (0 == SIGSTOP) ||
	    (0 == SIGTERM) ||
	    (0 == SIGTSTP) ||
	    (0 == SIGTTIN) ||
	    (0 == SIGTTOU) || (0 == SIGUSR1) || (0 == SIGUSR2) ||
#ifdef SIGPOLL
	    (0 == SIGPOLL) ||
#endif
#ifdef SIGPROF
	    (0 == SIGPROF) ||
#endif
	    (0 == SIGSYS) ||
	    (0 == SIGTRAP) ||
	    (0 == SIGURG) ||
	    (0 == SIGVTALRM) || (0 == SIGXCPU) || (0 == SIGXFSZ)) {
		printf("Test FAILED\n");
		return -1;
	} else {
		printf("Test PASSED\n");
		return 0;
	}
}
