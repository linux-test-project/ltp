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

static pid_t pid;

#ifdef __sparc__
/* sparce swaps addr/data for get/set regs */
# define maybe_swap(request, addr, data) \
do { \
	if (request == PTRACE_GETREGS || request == PTRACE_SETREGS) { \
		void *__s = addr; \
		addr = data; \
		data = __s; \
	} \
} while (0)
#else
# define maybe_swap(...)
#endif
#define vptrace(request, pid, addr, data) \
({ \
	errno = 0; \
	long __ret; \
	void *__addr = (void *)(addr); \
	void *__data = (void *)(data); \
	maybe_swap(request, __addr, __data); \
	__ret = ptrace(request, pid, __addr, __data); \
	if (__ret && errno) \
		perror("ptrace(" #request ", " #pid ", " #addr ", " #data ")"); \
	__ret; \
})

static void make_a_baby(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "child")) {
		/* if we're the child, just sit around doing nothing */
		int i = 60;
		while (i--) {
			close(-100);
			sleep(1);
		}
		exit(1);
	}

	signal(SIGCHLD, SIG_IGN);

	pid = vfork();
	if (pid == -1) {
		tst_resm(TFAIL, "vfork() failed");
		tst_exit();
	} else if (pid) {
		int status;

		if (wait(&status) != pid) {
			tst_brkm(TBROK | TERRNO, NULL, "wait(%i) failed: %#x", pid, status);
			kill(pid, SIGKILL);
			exit(1);
		}
		if (!WIFSTOPPED(status)) {
			tst_brkm(TBROK, NULL, "child status not stopped: %#x", status);
			kill(pid, SIGKILL);
			exit(1);
		}

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
#ifdef PTRACE_GETSIGINFO
	SPT(GETSIGINFO)
#endif
#ifdef PTRACE_SETSIGINFO
	SPT(SETSIGINFO)
#endif
#ifdef PTRACE_GETFGREGS
	SPT(GETFGREGS)
#endif
#ifdef PTRACE_SETFGREGS
	SPT(SETFGREGS)
#endif
	SPT(KILL)
	SPT(SINGLESTEP)
};
static inline char *strptrace(int request)
{
	return strings[request];
}
