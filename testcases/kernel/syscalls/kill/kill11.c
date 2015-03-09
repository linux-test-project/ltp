/* IBM Corporation
 * 01/02/2003	Port to LTP	avenkat@us.ibm.com
 * 06/30/2001	Port to Linux	nsharoff@us.ibm.com
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *   Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2014
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*

  Test check that when a child is killed by its parent, it returns the correct
  values to the waiting parent--default behaviour assumed by child.

 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "test.h"
#include "safe_macros.h"

#define FAILED 0
#define PASSED 1

char *TCID = "kill11";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
static int sig;

void setup(void);
void do_child(void);

/*
 * These signals terminate process by default, some create core file.
 */
struct tcase {
	int sig;
	int dumps_core;
} tcases[] = {
	{SIGHUP, 0},
	{SIGINT, 0},
	{SIGQUIT, 1},
	{SIGILL, 1},
	{SIGTRAP, 1},
	{SIGABRT, 1},
	{SIGIOT, 1},
	{SIGBUS, 1},
	{SIGFPE, 1},
	{SIGKILL, 0},
	{SIGUSR1, 0},
	{SIGSEGV, 1},
	{SIGUSR2, 0},
	{SIGPIPE, 0},
	{SIGALRM, 0},
	{SIGTERM, 0},
	{SIGXCPU, 1},
	{SIGXFSZ, 1},
	{SIGVTALRM, 0},
	{SIGPROF, 0},
	{SIGIO, 0},
	{SIGPWR, 0},
	{SIGSYS, 1},
};

static void verify_kill(struct tcase *t)
{
	int core;
	int pid, npid;
	int nsig, nexno, status;

	if (t->sig != SIGKILL) {
#ifndef BCS
		if (t->sig != SIGSTOP)
#endif
			if (sigset(t->sig, SIG_DFL) == SIG_ERR) {
				tst_brkm(TBROK | TERRNO, tst_rmdir,
				         "sigset(%d) failed", sig);
			}
	}

	pid = FORK_OR_VFORK();
	if (pid < 0)
		tst_brkm(TBROK | TERRNO, tst_rmdir, "fork() failed");

	if (pid == 0) {
#ifdef UCLINUX
		if (self_exec(argv[0], "dd", t->sig) < 0)
			exit(1);
#else
		do_child();
#endif
	}

	kill(pid, t->sig);
	npid = wait(&status);

	if (npid != pid) {
		tst_resm(TFAIL, "wait() returned %d, expected %d", npid, pid);
		return;
	}

	nsig = WTERMSIG(status);
#ifdef WCOREDUMP
	core = WCOREDUMP(status);
#endif
	nexno = WIFEXITED(status);

	if (t->dumps_core) {
		if (!core) {
			tst_resm(TFAIL, "core dump bit not set for %s", tst_strsig(t->sig));
			return;
		}
	} else {
		if (core) {
			tst_resm(TFAIL, "core dump bit set for %s", tst_strsig(t->sig));
			return;
		}
	}

	if (nsig != t->sig) {
		tst_resm(TFAIL, "wait: unexpected signal %d returned, expected %d", nsig, t->sig);
		return;
	}

	if (nexno != 0) {
		tst_resm(TFAIL,
			"signal: unexpected exit number %d returned, expected 0\n",
			nexno);
		return;
	}

	tst_resm(TPASS, "signal %-16s%s", tst_strsig(t->sig),
	         t->dumps_core ? " dumped core" : "");
}

int main(int argc, char **argv)
{
	int lc;
	unsigned int i;

	tst_parse_opts(argc, argv, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child, "dd", &sig);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < ARRAY_SIZE(tcases); i++)
			verify_kill(tcases + i);
	}

	tst_rmdir();
	tst_exit();
}

void do_child(void)
{
	int i;

	for (i = 0; i < 180; i++)
		sleep(1);

	fprintf(stderr, "Child missed siggnal");
	fflush(stderr);
	exit(1);
}

/* 1024 GNU blocks */
#define MIN_RLIMIT_CORE (1024 * 1024)

void setup(void)
{
	struct rlimit rlim;

	SAFE_GETRLIMIT(NULL, RLIMIT_CORE, &rlim);

	if (rlim.rlim_cur < MIN_RLIMIT_CORE) {
		tst_resm(TINFO, "Adjusting RLIMIT_CORE to %i", MIN_RLIMIT_CORE);
		rlim.rlim_cur = MIN_RLIMIT_CORE;
		SAFE_SETRLIMIT(NULL, RLIMIT_CORE, &rlim);
	}

	temp = stderr;
	tst_tmpdir();
}
