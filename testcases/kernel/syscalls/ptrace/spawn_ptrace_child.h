/*
 * Spawn a child and set it up for ptrace()-ing
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

/*
 * To use:
 *  - add this line after your normal includes:
 *       #include "spawn_ptrace_child.c"
 *  - add this line to the top of your main():
 *       make_a_baby(argc, argv);
 *  - access the child pid via the "pid" variable
 */

#include <errno.h>      /* errno */
#include <signal.h>     /* signal() */
#include <stdbool.h>    /* true */
#include <string.h>     /* strcmp() */
#include <unistd.h>     /* execlp() sleep() vfork() */
#include <sys/ptrace.h> /* ptrace() */
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

static pid_t pid;
static bool child_stopped = false;

static void child_signal(int sig)
{
	int status;
	if (sig != SIGCHLD) {
 die:
		tst_brkm(TBROK, tst_exit, "child signal %i: %s\n", sig, strsignal(sig));
		kill(pid, SIGKILL);
		exit(1);
	}
	if (wait(&status) == -1)
		goto die;
	if (!WIFSTOPPED(status))
		goto die;

	child_stopped = true;
}

static void make_a_baby(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "child")) {
		/* if we're the child, just sit around doing nothing */
		sleep(60);
		exit(1);
	}

	signal(SIGCHLD, child_signal);

	pid = vfork();
	if (pid == -1) {
		tst_resm(TFAIL, "vfork() failed");
		tst_exit();
	} else if (pid) {
		while (!child_stopped)
			continue;
		signal(SIGCHLD, SIG_IGN);
		return;
	}

	errno = 0;
	long ret = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	if (ret && errno) {
		tst_resm(TFAIL, "PTRACE_TRACEME failed");
		tst_exit();
	}

	execlp(argv[0], argv[0], "child", NULL);
	tst_resm(TFAIL, "execlp() failed");
	tst_exit();
}

#define SPT(x) [PTRACE_##x] = #x,
static char *strings[] = {
	SPT(TRACEME)
	SPT(PEEKTEXT)
	SPT(PEEKDATA)
	SPT(PEEKUSER)
	SPT(POKETEXT)
	SPT(POKEDATA)
	SPT(POKEUSER)
#ifdef PTRACE_GETREGS
	SPT(GETREGS)
#endif
#ifdef PTRACE_SETREGS
	SPT(SETREGS)
#endif
	SPT(GETSIGINFO)
	SPT(SETSIGINFO)
#ifdef PTRACE_GETFGREGS
	SPT(GETFGREGS)
#endif
#ifdef PTRACE_SETFGREGS
	SPT(SETFGREGS)
#endif
	SPT(KILL)
	SPT(SINGLESTEP)
};
static char *strptrace(enum __ptrace_request request)
{
	return strings[request];
}
