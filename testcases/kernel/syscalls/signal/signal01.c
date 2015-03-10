/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR           :  Dave Baumgartner
 *                   :  Rewrote 12/92 by Richard Logan
 *  CO-PILOT         :  Barrie Kletscher
 *  DATE STARTED     :  10/17/85
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
 *
 */
/*
 * TEST ITEMS
 *
 *	1. SIGKILL can not be set to be caught, errno:EINVAL (POSIX).
 *	2. SIGKILL can not be caught.
 *	3. SIGKILL can not be set to be ignored, errno:EINVAL (POSIX).
 *	4. SIGKILL can not be ignored.
 *	5. SIGKILL can not be reset to default, errno:EINVAL (POSIX).
 */
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void do_test(int tc);
static void do_child(void);
static void catchsig(int sig);

static struct tcase {
	void (*sighandler)(int);
	int kill;
} tcases[] = {
	{SIG_IGN, 0},
	{SIG_DFL, 0},
	{catchsig, 0},
	{SIG_IGN, 1},
	{SIG_DFL, 1},
	{catchsig, 1},
};

char *TCID = "signal01";
int TST_TOTAL = ARRAY_SIZE(tcases);

static int tcase;

int main(int argc, char *argv[])
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child, "d", &tcase);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++)
			do_test(i);
	}

	tst_exit();
}

static void do_test(int tc)
{
	pid_t pid;
	int res;
	pid = FORK_OR_VFORK();

	switch (pid) {
	case 0:
#ifdef UCLINUX
		if (self_exec(argv0, "d", tc) < 0)
			tst_brkm(TBROK | TERRNO, NULL, "self_exec() failed");
#else
		tcase = tc;
		do_child();
#endif
	break;
	case -1:
		tst_resm(TBROK | TERRNO, "fork() failed");
	break;
	default:
		if (tcases[tc].kill) {
			TST_PROCESS_STATE_WAIT(NULL, pid, 'S');

			SAFE_KILL(NULL, pid, SIGKILL);

			SAFE_WAITPID(NULL, pid, &res, 0);

			if (WIFSIGNALED(res)) {
				if (WTERMSIG(res) == SIGKILL) {
					tst_resm(TPASS, "Child killed with SIGKILL");
				} else {
					tst_resm(TFAIL, "Child killed with %s",
					         tst_strsig(WTERMSIG(res)));
				}
			} else {
				tst_resm(TFAIL, "Child not killed by signal");
			}
		} else {
			tst_record_childstatus(NULL, pid);
		}
	break;
	}
}

static void catchsig(int sig)
{
	(void)sig;
}

static const char *strhandler(void *sighandler)
{
	switch ((long)sighandler) {
	case (long)SIG_DFL:
		return "SIG_DFL";
	case (long)SIG_IGN:
		return "SIG_IGN";
	default:
		return "catchsig()";
	}
}

static void do_child(void)
{
	void *ret;
	void (*sighandler)(int) = tcases[tcase].sighandler;

	ret = signal(SIGKILL, sighandler);

	if (tcases[tcase].kill)
		pause();

	if (ret == SIG_ERR || errno == EINVAL) {
		tst_resm(TPASS, "signal(SIGKILL, %p(%s)) failed with EINVAL",
		         sighandler, strhandler(sighandler));
	} else {
		if (ret != SIG_ERR) {
			tst_resm(TFAIL, "signal(SIGKILL, %p(%s)) didn't fail",
			         sighandler, strhandler(sighandler));
		} else {
			tst_resm(TFAIL | TERRNO,
			         "signal(SIGKILL, %p(%s)) should fail with EINVAL",
			          sighandler, strhandler(sighandler));
		}
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	TEST_PAUSE;
}
