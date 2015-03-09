/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include "test.h"
#include "clone_platform.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"

char *TCID = "clone08";

static pid_t ptid, ctid, tgid;
static void *child_stack;

static void setup(void);
static void cleanup(void);

static void test_clone_parent(int t);
static int child_clone_parent(void);
static pid_t parent_ppid;

static void test_clone_tid(int t);
static int child_clone_child_settid(void);
static int child_clone_parent_settid(void);

#ifdef CLONE_STOPPED
static void test_clone_stopped(int t);
static int child_clone_stopped(void);
static int stopped_flag;
#endif

static void test_clone_thread(int t);
static int child_clone_thread(void);
static int tst_result;

/*
 * Children cloned with CLONE_VM should avoid using any functions that
 * might require dl_runtime_resolve, because they share thread-local
 * storage with parent. If both try to resolve symbols at same time you
 * can crash, likely at _dl_x86_64_restore_sse().
 * See this thread for relevant discussion:
 * http://www.mail-archive.com/utrace-devel@redhat.com/msg01944.html
 */
static struct test_case {
	char *name;
	int flags;
	void (*testfunc)(int);
	int (*do_child)();
} test_cases[] = {
	{"CLONE_PARENT", CLONE_PARENT | SIGCHLD,
	 test_clone_parent, child_clone_parent},
	{"CLONE_CHILD_SETTID", CLONE_CHILD_SETTID | SIGCHLD,
	 test_clone_tid, child_clone_child_settid},
	{"CLONE_PARENT_SETTID", CLONE_PARENT_SETTID | CLONE_VM | SIGCHLD,
	 test_clone_tid, child_clone_parent_settid},
#ifdef CLONE_STOPPED
	{"CLONE_STOPPED", CLONE_STOPPED | CLONE_VM | SIGCHLD,
	 test_clone_stopped, child_clone_stopped},
#endif
	{"CLONE_THREAD", CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | SIGCHLD,
	 test_clone_thread, child_clone_thread},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
			tst_resm(TINFO, "running %s", test_cases[i].name);
			test_cases[i].testfunc(i);
		}
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	child_stack = SAFE_MALLOC(cleanup, CHILD_STACK_SIZE);
}

static void cleanup(void)
{
	free(child_stack);

	tst_rmdir();
}

static long clone_child(const struct test_case *t, int use_tst)
{
	TEST(ltp_clone7(t->flags, t->do_child, NULL, CHILD_STACK_SIZE,
		child_stack, &ptid, NULL, &ctid));

	if (TEST_RETURN == -1 && TTERRNO == ENOSYS)
		tst_brkm(TCONF, cleanup, "clone does not support 7 args");

	if (TEST_RETURN == -1) {
		if (use_tst) {
			tst_brkm(TBROK | TTERRNO, cleanup, "%s clone() failed",
				 t->name);
		} else {
			printf("%s clone() failed, errno: %d",
			       t->name, TEST_ERRNO);
			exit(1);
		}
	}
	return TEST_RETURN;
}

static int wait4child(pid_t child)
{
	int status;

	if (waitpid(child, &status, 0) == -1)
		tst_resm(TBROK|TERRNO, "waitpid");
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return status;
}

static void test_clone_parent(int t)
{
	int status;
	pid_t child;

	fflush(stdout);
	child = FORK_OR_VFORK();
	switch (child) {
	case 0:
		parent_ppid = getppid();
		clone_child(&test_cases[t], 0);
		exit(0);
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "test_clone_parent fork");
	default:
		status = wait4child(child);
		if (status == 0) {
			/* wait for CLONE_PARENT child */
			status = wait4child(-1);
			if (status == 0) {
				tst_resm(TPASS, "test %s", test_cases[t].name);
			} else {
				tst_resm(TFAIL, "test %s, status: %d",
					 test_cases[t].name, status);
			}
		} else {
			tst_resm(TFAIL, "test %s, status: %d",
				 test_cases[t].name, status);
		}
	};
}

static int child_clone_parent(void)
{
	if (parent_ppid == getppid())
		exit(0);
	printf("FAIL: getppid != parent_ppid (%d != %d)\n",
	       parent_ppid, getppid());
	exit(1);
}

static void test_clone_tid(int t)
{
	int status;
	pid_t child;

	child = clone_child(&test_cases[t], 1);
	status = wait4child(child);
	if (status == 0) {
		tst_resm(TPASS, "test %s", test_cases[t].name);
	} else {
		tst_resm(TFAIL, "test %s, status: %d",
			 test_cases[t].name, status);
	}
}

static int child_clone_child_settid(void)
{
	if (ctid == ltp_syscall(__NR_getpid))
		ltp_syscall(__NR_exit, 0);
	printf("FAIL: ctid != getpid() (%d != %d)\n",
	       ctid, getpid());
	ltp_syscall(__NR_exit, 1);
	return 0;
}

static int child_clone_parent_settid(void)
{
	if (ptid == ltp_syscall(__NR_getpid))
		ltp_syscall(__NR_exit, 0);
	printf("FAIL: ptid != getpid() (%d != %d)\n",
	       ptid, getpid());
	ltp_syscall(__NR_exit, 1);
	return 0;
}

#ifdef CLONE_STOPPED
static void test_clone_stopped(int t)
{
	int i;
	int status;
	int flag;
	pid_t child;

	if (tst_kvercmp(2, 6, 38) >= 0) {
		tst_resm(TINFO, "CLONE_STOPPED skipped for kernels >= 2.6.38");
		return;
	}

	stopped_flag = 0;
	child = clone_child(&test_cases[t], 1);

	/* give the kernel scheduler chance to run the CLONE_STOPPED thread*/
	for (i = 0; i < 100; i++) {
		sched_yield();
		usleep(1000);
	}

	flag = stopped_flag;
	if (kill(child, SIGCONT) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "kill SIGCONT failed");

	status = wait4child(child);
	if (status == 0 && flag == 0) {
		tst_resm(TPASS, "test %s", test_cases[t].name);
	} else {
		tst_resm(TFAIL, "test %s, status: %d, flag: %d",
			 test_cases[t].name, status, flag);
	}
}

static int child_clone_stopped(void)
{
	stopped_flag = 1;
	ltp_syscall(__NR_exit, 0);
	return 0;
}
#endif

static void test_clone_thread(int t)
{
	pid_t child;
	int i, status;

	fflush(stdout);
	child = FORK_OR_VFORK();
	switch (child) {
	case 0:
		tgid = ltp_syscall(__NR_getpid);
		tst_result = -1;
		clone_child(&test_cases[t], 0);

		for (i = 0; i < 5000; i++) {
			sched_yield();
			usleep(1000);
			if (tst_result != -1)
				break;
		}
		ltp_syscall(__NR_exit, tst_result);
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "test_clone_thread fork");
	default:
		status = wait4child(child);
		if (status == 0) {
			tst_resm(TPASS, "test %s", test_cases[t].name);
		} else {
			tst_resm(TFAIL, "test %s, status: %d",
				 test_cases[t].name, status);
		}
	};
}

static int child_clone_thread(void)
{
	if (tgid == ltp_syscall(__NR_getpid))
		tst_result = TPASS;
	else
		tst_result = TFAIL;
	ltp_syscall(__NR_exit, 0);
	return 0;
}
