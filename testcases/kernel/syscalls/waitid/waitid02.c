/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007                                   */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License along    */
/* with this program; if not, write to the Free Software Foundation,  Inc.,   */
/* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA                 */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        waitid02.c                                           	      */
/*                                                                            */
/* Description: This tests the waitid() syscall                               */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* waitid02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                              */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   waitid02                                                      */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

struct testcase_t {
	const char *msg;
	idtype_t idtype;
	id_t id;
	pid_t child;
	int options;
	int exp_ret;
	int exp_errno;
	void (*setup) (struct testcase_t *);
	void (*cleanup) (struct testcase_t *);
};

static void setup(void);
static void cleanup(void);

static void setup2(struct testcase_t *);
static void setup3(struct testcase_t *);
static void setup4(struct testcase_t *);
static void setup5(struct testcase_t *);
static void setup6(struct testcase_t *);
static void cleanup2(struct testcase_t *);
static void cleanup5(struct testcase_t *);
static void cleanup6(struct testcase_t *);

struct testcase_t tdat[] = {
	{
		.msg = "WNOHANG",
		.idtype = P_ALL,
		.id = 0,
		.options = WNOHANG,
		.exp_ret = -1,
		.exp_errno = EINVAL,
	},
	{
		.msg = "WNOHANG | WEXITED no child",
		.idtype = P_ALL,
		.id = 0,
		.options = WNOHANG | WEXITED,
		.exp_ret = -1,
		.exp_errno = ECHILD,
	},
	{
		.msg = "WNOHANG | WEXITED with child",
		.idtype = P_ALL,
		.id = 0,
		.options = WNOHANG | WEXITED,
		.exp_ret = 0,
		.setup = setup2,
		.cleanup = cleanup2
	},
	{
		.msg = "P_PGID, WEXITED wait for child",
		.idtype = P_PGID,
		.options = WEXITED,
		.exp_ret = 0,
		.setup = setup3,
	},
	{
		.msg = "P_PID, WEXITED wait for child",
		.idtype = P_PID,
		.options = WEXITED,
		.exp_ret = 0,
		.setup = setup4,
	},
	{
		.msg = "P_PID, WSTOPPED | WNOWAIT",
		.idtype = P_PID,
		.options = WSTOPPED | WNOWAIT,
		.exp_ret = 0,
		.setup = setup5,
		.cleanup = cleanup5
	},
	{
		.msg = "P_PID, WCONTINUED",
		.idtype = P_PID,
		.options = WCONTINUED,
		.exp_ret = 0,
		.setup = setup6,
		.cleanup = cleanup6
	},
	{
		.msg = "P_PID, WEXITED not a child of the calling process",
		.idtype = P_PID,
		.id = 1,
		.options = WEXITED,
		.exp_ret = -1,
		.exp_errno = ECHILD,
		.setup = setup2,
		.cleanup = cleanup2
	},

};

char *TCID = "waitid02";
static int TST_TOTAL = ARRAY_SIZE(tdat);

static void makechild(struct testcase_t *t, void (*childfn)(void))
{
	t->child = fork();
	switch (t->child) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
		break;
	case 0:
		childfn();
		exit(0);
	}
}

static void wait4child(pid_t pid)
{
	int status;
	SAFE_WAITPID(cleanup, pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "child returns %d", status);
}

static void dummy_child(void)
{
}

static void waiting_child(void)
{
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);
}

static void stopped_child(void)
{
	kill(getpid(), SIGSTOP);
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);
}

static void setup2(struct testcase_t *t)
{
	makechild(t, waiting_child);
}

static void cleanup2(struct testcase_t *t)
{
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
	wait4child(t->child);
}

static void setup3(struct testcase_t *t)
{
	t->id = getpgid(0);
	makechild(t, dummy_child);
}

static void setup4(struct testcase_t *t)
{
	makechild(t, dummy_child);
	t->id = t->child;
}

static void setup5(struct testcase_t *t)
{
	makechild(t, stopped_child);
	t->id = t->child;
}

static void cleanup5(struct testcase_t *t)
{
	kill(t->child, SIGCONT);
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
	wait4child(t->child);
}

static void setup6(struct testcase_t *t)
{
	siginfo_t infop;
	makechild(t, stopped_child);
	t->id = t->child;
	if (waitid(P_PID, t->child, &infop, WSTOPPED) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "waitpid setup6");
	kill(t->child, SIGCONT);
}

static void cleanup6(struct testcase_t *t)
{
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
	wait4child(t->child);
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
	TST_CHECKPOINT_INIT(tst_rmdir);
}

static void cleanup(void)
{
	tst_rmdir();
	tst_exit();
}

static void test_waitid(struct testcase_t *t)
{
	siginfo_t infop;

	if (t->setup)
		t->setup(t);

	tst_resm(TINFO, "%s", t->msg);
	tst_resm(TINFO, "(%d) waitid(%d, %d, %p, %d)", getpid(), t->idtype,
			t->id, &infop, t->options);
	memset(&infop, 0, sizeof(infop));

	TEST(waitid(t->idtype, t->id, &infop, t->options));
	if (TEST_RETURN == t->exp_ret) {
		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == t->exp_errno)
				tst_resm(TPASS, "exp_errno=%d", t->exp_errno);
			else
				tst_resm(TFAIL|TTERRNO, "exp_errno=%d",
					t->exp_errno);
		} else {
			tst_resm(TPASS, "ret: %d", t->exp_ret);
		}
	} else {
		tst_resm(TFAIL|TTERRNO, "ret=%ld expected=%d",
			TEST_RETURN, t->exp_ret);
	}
	tst_resm(TINFO, "si_pid = %d ; si_code = %d ; si_status = %d",
			infop.si_pid, infop.si_code,
			infop.si_status);

	if (t->cleanup)
		t->cleanup(t);
}

int main(int ac, char **av)
{
	int lc, testno;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; testno++)
			test_waitid(&tdat[testno]);
	}
	cleanup();
	tst_exit();
}
