// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "clone_platform.h"
#include "lapi/syscalls.h"
#include "lapi/futex.h"

static pid_t ptid, ctid, tgid;
static void *child_stack;

static void test_clone_parent(int t);
static int child_clone_parent(void *);
static pid_t parent_ppid;

static void test_clone_tid(int t);
static int child_clone_child_settid(void *);
static int child_clone_parent_settid(void *);


static void test_clone_thread(int t);
static int child_clone_thread(void *);

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
	int (*do_child)(void *);
} test_cases[] = {
	{"CLONE_PARENT", CLONE_PARENT | SIGCHLD,
	 test_clone_parent, child_clone_parent},
	{"CLONE_CHILD_SETTID", CLONE_CHILD_SETTID | SIGCHLD,
	 test_clone_tid, child_clone_child_settid},
	{"CLONE_PARENT_SETTID", CLONE_PARENT_SETTID | CLONE_VM | SIGCHLD,
	 test_clone_tid, child_clone_parent_settid},
	{"CLONE_THREAD", CLONE_THREAD | CLONE_SIGHAND | CLONE_VM |
	 CLONE_CHILD_CLEARTID | SIGCHLD,
	 test_clone_thread, child_clone_thread},
};

static void do_test(unsigned int i)
{
	tst_res(TINFO, "running %s", test_cases[i].name);
	test_cases[i].testfunc(i);
}

static void setup(void)
{
	child_stack = SAFE_MALLOC(CHILD_STACK_SIZE);
}

static void cleanup(void)
{
	free(child_stack);
}

static long clone_child(const struct test_case *t)
{
	TEST(ltp_clone7(t->flags, t->do_child, NULL, CHILD_STACK_SIZE,
		child_stack, &ptid, NULL, &ctid));

	if (TST_RET == -1 && TTERRNO == ENOSYS)
		tst_brk(TCONF, "clone does not support 7 args");

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "%s clone() failed", t->name);

	return TST_RET;
}

static void test_clone_parent(int t)
{
	pid_t child;

	child = SAFE_FORK();
	if (!child) {
		parent_ppid = getppid();
		clone_child(&test_cases[t]);
		_exit(0);
	}
	tst_reap_children();
}

static int child_clone_parent(void *arg LTP_ATTRIBUTE_UNUSED)
{
	if (parent_ppid == getppid()) {
		tst_res(TPASS, "clone and forked child has the same parent");
	} else {
		tst_res(TFAIL, "getppid != parent_ppid (%d != %d)",
			parent_ppid, getppid());
	}
	tst_syscall(__NR_exit, 0);
	return 0;
}

static void test_clone_tid(int t)
{
	clone_child(&test_cases[t]);
	tst_reap_children();
}

static int child_clone_child_settid(void *arg LTP_ATTRIBUTE_UNUSED)
{
	if (ctid == tst_syscall(__NR_getpid))
		tst_res(TPASS, "clone() correctly set ctid");
	else
		tst_res(TFAIL, "ctid != getpid() (%d != %d)", ctid, getpid());
	tst_syscall(__NR_exit, 0);
	return 0;
}

static int child_clone_parent_settid(void *arg LTP_ATTRIBUTE_UNUSED)
{
	if (ptid == tst_syscall(__NR_getpid))
		tst_res(TPASS, "clone() correctly set ptid");
	else
		tst_res(TFAIL, "ptid != getpid() (%d != %d)", ptid, getpid());
	tst_syscall(__NR_exit, 0);
	return 0;
}

static void test_clone_thread(int t)
{
	pid_t child;

	child = SAFE_FORK();
	if (!child) {
		struct timespec timeout = { 5 /* sec */, 0 };

		tgid = tst_syscall(__NR_getpid);
		ctid = -1;

		clone_child(&test_cases[t]);

		if (syscall(SYS_futex, &ctid, FUTEX_WAIT, -1, &timeout)) {
			/*
			 * futex here is racing with clone() above.
			 * Which means we can get EWOULDBLOCK if
			 * ctid has been already changed by clone()
			 * before we make the call. As long as ctid
			 * changes we should not report error when
			 * futex returns EWOULDBLOCK.
			 */
			if (errno != EWOULDBLOCK || ctid == -1) {
				tst_res(TFAIL | TERRNO,
					"futex failed, ctid: %d", ctid);
				_exit(0);
			}
		}
		tst_res(TPASS, "futex exit on ctid change, ctid: %d", ctid);
		_exit(0);
	}

	tst_reap_children();
}

static int child_clone_thread(void *arg LTP_ATTRIBUTE_UNUSED)
{
	if (tgid == tst_syscall(__NR_getpid))
		tst_res(TPASS, "clone has the same thread id");
	else
		tst_res(TFAIL, "clone's thread id not equal parent's id");
	tst_syscall(__NR_exit, 0);
	return 0;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_cases),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1
};
