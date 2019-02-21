/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR          : Bob Clark
 *  CO-PILOT        : Barrie Kletscher
 *  DATE STARTED    : 9/26/86
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
/*
 * TEST ITEMS
 *	1. sighold action to turn off the receipt of all signals was done
 *	   without error.
 *	2. After signals were held, and sent, no signals were trapped.
 */
#define _XOPEN_SOURCE 500
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/signal.h"

/* _XOPEN_SOURCE disables NSIG */
#ifndef NSIG
# define NSIG _NSIG
#endif

/* ensure NUMSIGS is defined */
#ifndef NUMSIGS
# define NUMSIGS NSIG
#endif

char *TCID = "sighold02";
int TST_TOTAL = 2;

static int pid;
static void do_child(void);
static void setup(void);
static void cleanup(void);

static int sigs_catched;
static int sigs_map[NUMSIGS];

static int skip_sig(int sig)
{
	if (sig >= __SIGRTMIN && sig < SIGRTMIN)
		return 1;

	switch (sig) {
	case SIGCHLD:
	case SIGKILL:
	case SIGALRM:
	case SIGSTOP:
		return 1;
	default:
		return 0;
	}
}

int main(int ac, char **av)
{
	int sig;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK | TERRNO, NULL, "fork() failed");
		} else if (pid > 0) {
			TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

			for (sig = 1; sig < NUMSIGS; sig++) {
				if (skip_sig(sig))
					continue;
				SAFE_KILL(NULL, pid, sig);
			}

			TST_SAFE_CHECKPOINT_WAKE(NULL, 0);
			tst_record_childstatus(cleanup, pid);
		} else {

#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK | TERRNO, NULL,
					 "self_exec() failed");
			}
#else
			do_child();
#endif
		}
	}

	cleanup();
	tst_exit();
}

static void handle_sigs(int sig)
{
	sigs_map[sig] = 1;
	sigs_catched++;
}

void do_child(void)
{
	int cnt;
	int sig;

	/* set up signal handler routine */
	for (sig = 1; sig < NUMSIGS; sig++) {
		if (skip_sig(sig))
			continue;

		if (signal(sig, handle_sigs) == SIG_ERR) {
			tst_resm(TBROK | TERRNO, "signal() %i(%s) failed",
				 sig, tst_strsig(sig));
		}
	}

	/* all set up to catch signals, now hold them */
	for (cnt = 0, sig = 1; sig < NUMSIGS; sig++) {
		if (skip_sig(sig))
			continue;
		cnt++;
		TEST(sighold(sig));
		if (TEST_RETURN != 0) {
			tst_resm(TBROK | TTERRNO, "sighold() %i(%s) failed",
				 sig, tst_strsig(sig));
		}
	}

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	if (!sigs_catched) {
		tst_resm(TPASS, "All signals were hold");
		tst_exit();
	}

	tst_resm(TFAIL, "Signal handler was executed");

	for (sig = 1; sig < NUMSIGS; sig++) {
		if (sigs_map[sig]) {
			tst_resm(TINFO, "Signal %i(%s) catched",
			         sig, tst_strsig(sig));
		}
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	tst_tmpdir();

	TST_CHECKPOINT_INIT(tst_rmdir);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
