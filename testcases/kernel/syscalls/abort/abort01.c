/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *   01/02/2003	Port to LTP	avenkat@us.ibm.com
 *   11/11/2002: Ported to LTP Suite by Ananda
 *   06/30/2001	Port to Linux	nsharoff@us.ibm.com
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

 /* ALGORITHM
 *	Fork child.  Have child abort, check return status.
 *
 * RESTRICTIONS
 *      The ulimit for core file size must be greater than 0.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

#include "test.h"
#include "safe_macros.h"

#define NUM 3

char *TCID = "abort01";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);
static void do_child();
static int instress();

int main(int argc, char *argv[])
{
	register int i;
	int status, count, child, kidpid;
	int sig, ex;

#ifdef WCOREDUMP
	int core;
	core = 0;
#endif
	ex = sig = 0;

	tst_parse_opts(argc, argv, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (i = 0; i < NUM; i++) {
		kidpid = FORK_OR_VFORK();
		if (kidpid == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "")) {
				if (!instress()) {
					perror("fork failed");
					exit(1);
				}
			}
#else
			do_child();
#endif
		}
		if (kidpid < 0)
			if (!instress())
				tst_brkm(TBROK | TERRNO, cleanup,
					 "fork failed");
		count = 0;
		while ((child = wait(&status)) > 0)
			count++;
		if (count != 1) {
			tst_brkm(TBROK, cleanup,
				 "wrong # children waited on; got %d, expected 1",
				 count);
		}
		if (WIFSIGNALED(status)) {

#ifdef WCOREDUMP
			core = WCOREDUMP(status);
#endif
			sig = WTERMSIG(status);

		}
		if (WIFEXITED(status))
			ex = WEXITSTATUS(status);

#ifdef WCOREDUMP
		if (core == 0) {
			tst_brkm(TFAIL, cleanup,
				 "Child did not dump core; exit code = %d, "
				 "signal = %d", ex, sig);
		} else if (core != -1) {
			tst_resm(TPASS, "abort dumped core");
		}
#endif
		if (sig == SIGIOT) {
			tst_resm(TPASS, "abort raised SIGIOT");
		} else {
			tst_brkm(TFAIL, cleanup,
				 "Child did not raise SIGIOT (%d); exit code = %d, "
				 "signal = %d", SIGIOT, ex, sig);
		}

	}

	cleanup();
	tst_exit();
}

/* 1024 GNU blocks */
#define MIN_RLIMIT_CORE (1024 * 1024)

static void setup(void)
{
	struct rlimit rlim;

	SAFE_GETRLIMIT(NULL, RLIMIT_CORE, &rlim);

	if (rlim.rlim_cur < MIN_RLIMIT_CORE) {
		tst_resm(TINFO, "Adjusting RLIMIT_CORE to %i", MIN_RLIMIT_CORE);
		rlim.rlim_cur = MIN_RLIMIT_CORE;
		SAFE_SETRLIMIT(NULL, RLIMIT_CORE, &rlim);
	}

	tst_tmpdir();
}

static void cleanup(void)
{
	unlink("core");
	tst_rmdir();
}

static void do_child(void)
{
	abort();
	fprintf(stderr, "\tchild - abort failed.\n");
	exit(1);
}

static int instress(void)
{
	tst_resm(TINFO,
		 "System resources may be too low; fork(), select() etc are likely to fail.");
	return 1;
}
