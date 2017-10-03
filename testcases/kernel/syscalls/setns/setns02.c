/*
 * Copyright (C) 2013 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * functional test for setns(2) - reassociate thread with a namespace
 * 1. create child with CLONE_NEWUTS, set different hostname in child,
 *    set namespace back to parent ns and check that hostname has changed
 * 2. create child with CLONE_NEWIPC, set up shared memory in parent
 *    and verify that child can't shmat it, then set namespace
 *    back to parent one and verify that child is able to do shmat
 */
#define _GNU_SOURCE
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include "config.h"
#include "test.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"

#define CHILD_STACK_SIZE (1024*1024)
#define CP "(child) "
char *TCID = "setns02";

#if defined(__NR_setns) && defined(CLONE_NEWIPC) && defined(CLONE_NEWUTS)
#include "setns.h"

static char *dummy_hostname = "setns_dummy_uts";
static int ns_ipc_fd;
static int ns_uts_fd;
static key_t ipc_key;
static int shmid;

static void setup(void);
static void cleanup(void);

static int do_child_newuts(void *arg)
{
	struct utsname uts, uts_parent;
	int ns_flag = *(int *)arg;

	if (uname(&uts_parent) == -1)
		tst_resm(TFAIL|TERRNO, CP"uname");
	tst_resm(TINFO, CP"hostname (inherited from parent): %s",
		uts_parent.nodename);

	if (sethostname(dummy_hostname, strlen(dummy_hostname)) == -1)
		tst_resm(TFAIL|TERRNO, CP"sethostname");
	if (uname(&uts) == -1)
		tst_resm(TFAIL|TERRNO, CP"uname");

	tst_resm(TINFO, CP"hostname changed to: %s", uts.nodename);
	if (strcmp(uts_parent.nodename, uts.nodename) == 0) {
		tst_resm(TFAIL, CP"expected hostname to be different");
		return 1;
	} else {
		tst_resm(TPASS, CP"hostname is different in parent/child");
	}

	tst_resm(TINFO, CP"attempting to switch ns back to parent ns");
	if (syscall(__NR_setns, ns_uts_fd, ns_flag) == -1) {
		tst_resm(TFAIL|TERRNO, CP"setns");
		return 2;
	}
	if (uname(&uts) == -1)
		tst_resm(TFAIL|TERRNO, CP"uname");

	tst_resm(TINFO, CP"hostname: %s", uts.nodename);
	if (strcmp(uts_parent.nodename, uts.nodename) != 0) {
		tst_resm(TFAIL, CP"expected hostname to match parent");
		return 3;
	} else {
		tst_resm(TPASS, CP"hostname now as expected");
	}
	return 0;
}

static int do_child_newipc(void *arg)
{
	void *p;
	int ns_flag = *(int *)arg;

	p = shmat(shmid, NULL, 0);
	if (p == (void *) -1) {
		tst_resm(TPASS|TERRNO, CP"shmat failed as expected");
	} else {
		tst_resm(TFAIL, CP"shmat unexpectedly suceeded");
		shmdt(p);
		return 1;
	}

	tst_resm(TINFO, CP"attempting to switch ns back to parent ns");
	if (syscall(__NR_setns, ns_ipc_fd, ns_flag) == -1) {
		tst_resm(TFAIL|TERRNO, CP"setns");
		return 2;
	}

	p = shmat(shmid, NULL, 0);
	if (p == (void *) -1) {
		tst_resm(TFAIL|TERRNO, CP"shmat failed after setns");
		return 3;
	} else {
		tst_resm(TPASS, CP"shmat suceeded");
		shmdt(p);
	}

	return 0;
}

static void test_flag(int clone_flag, int ns_flag, int (*fn) (void *arg))
{
	void *child_stack;
	int ret, status;

	child_stack = malloc(CHILD_STACK_SIZE);
	if (child_stack == NULL)
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");

	tst_resm(TINFO, "creating child with clone_flag=0x%x, ns_flag=0x%x",
		clone_flag, ns_flag);
	ret = ltp_clone(SIGCHLD|clone_flag, fn, &ns_flag,
		CHILD_STACK_SIZE, child_stack);
	if (ret == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "ltp_clone");

	SAFE_WAITPID(cleanup, ret, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "child returns %d", status);
	else
		tst_resm(TPASS, "child finished succesfully");
	free(child_stack);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		if (ns_uts_fd != -1) {
			tst_resm(TINFO, "test_newuts");
			test_flag(CLONE_NEWUTS, CLONE_NEWUTS, do_child_newuts);
			test_flag(CLONE_NEWUTS, 0, do_child_newuts);
		}
		if (ns_ipc_fd != -1) {
			tst_resm(TINFO, "test_newipc");
			test_flag(CLONE_NEWIPC, CLONE_NEWIPC, do_child_newipc);
			test_flag(CLONE_NEWIPC, 0, do_child_newipc);
		}
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	char tmp[PATH_MAX];

	tst_require_root();

	/* runtime check if syscall is supported */
	ltp_syscall(__NR_setns, -1, 0);

	/* check if kernel has CONFIG_*_NS set and exports /proc entries */
	ns_ipc_fd = get_ns_fd(getpid(), "ipc");
	ns_uts_fd = get_ns_fd(getpid(), "uts");
	if (ns_ipc_fd == -1 && ns_uts_fd == -1)
		tst_brkm(TCONF, NULL, "your kernel has CONFIG_IPC_NS, "
			"CONFIG_UTS_NS or CONFIG_PROC disabled");

	if (getcwd(tmp, PATH_MAX) == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getcwd");
	ipc_key = ftok(tmp, 65);
	shmid = shmget(ipc_key, getpagesize(), IPC_CREAT | 0666);
	if (shmid == -1)
		tst_brkm(TBROK|TERRNO, NULL, "shmget");

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (ns_ipc_fd != -1)
		close(ns_ipc_fd);
	if (ns_uts_fd != -1)
		close(ns_uts_fd);

	shmctl(shmid, IPC_RMID, NULL);
}
#else
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "__NR_setns, CLONE_NEWIPC or CLONE_NEWUTS "
		" is not defined on your system.");
}
#endif
