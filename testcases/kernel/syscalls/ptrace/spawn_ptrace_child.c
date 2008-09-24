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
#include <string.h>     /* strcmp() */
#include <unistd.h>     /* execlp() sleep() vfork() */
#include <sys/ptrace.h> /* ptrace() */
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

static pid_t pid;

static void make_a_baby(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "child")) {
		/* if we're the child, just sit around doing nothing */
		while (1)
			sleep(1);
	}

	/* ptrace() stuff will fail for us if the child dies */
	signal(SIGCHLD, SIG_IGN);

	pid = vfork();
	if (pid == -1) {
		tst_resm(TFAIL, "vfork() failed");
		tst_exit();
	} else if (pid)
		return;

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
