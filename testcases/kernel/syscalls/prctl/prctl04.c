// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test PR_GET_SECCOMP and PR_SET_SECCOMP of prctl(2).
 *
 * - If PR_SET_SECCOMP sets the SECCOMP_MODE_STRICT for the calling thread,
 *   the only system call that the thread is permitted to make are read(2),
 *   write(2),_exit(2)(but not exit_group(2)), and sigreturn(2).  Other
 *   system calls result in the delivery of a SIGKILL signal. This operation
 *   is available only if the kernel is configured with CONFIG_SECCOMP enabled.
 *
 * - If PR_SET_SECCOMP sets the SECCOMP_MODE_FILTER for the calling thread,
 *   the system calls allowed are defined by a pointer to a Berkeley Packet
 *   Filter. Other system calls result int the delivery of a SIGSYS signal
 *   with SECCOMP_RET_KILL. The SECCOMP_SET_MODE_FILTER operation is available
 *   only if the kernel is configured with CONFIG_SECCOMP_FILTER enabled.
 *
 * - If SECCOMP_MODE_FILTER filters permit fork(2), then the seccomp mode
 *   is inherited by children created by fork(2).
 */

#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/filter.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/prctl.h"
#include "config.h"
#include "lapi/seccomp.h"

#define FNAME "filename"

static const struct sock_filter  strict_filter[] = {
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),

	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_waitid, 7, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_rt_sigprocmask, 6, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_close, 5, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_exit,  4, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_wait4, 3, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_write, 2, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ, __NR_clone, 1, 0),

	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)
};

static const struct sock_fprog  strict = {
	.len = (unsigned short)ARRAY_SIZE(strict_filter),
	.filter = (struct sock_filter *)strict_filter
};

static void check_strict_mode(int);
static void check_filter_mode(int);

static struct tcase {
	void (*func_check)();
	int pass_flag;
	int val;
	int exp_signal;
	char *message;
} tcases[] = {
	{check_strict_mode, 1, 1, SIGKILL,
	"SECCOMP_MODE_STRICT doesn't permit GET_SECCOMP call"},

	{check_strict_mode, 0, 2, SIGKILL,
	"SECCOMP_MODE_STRICT doesn't permit read(2) write(2) and _exit(2)"},

	{check_strict_mode, 1, 3, SIGKILL,
	"SECCOMP_MODE_STRICT doesn't permit close(2)"},

	{check_filter_mode, 1, 1, SIGSYS,
	"SECCOMP_MODE_FILTER doestn't permit GET_SECCOMP call"},

	{check_filter_mode, 0, 2, SIGSYS,
	"SECCOMP_MODE_FILTER doesn't permit close(2)"},

	{check_filter_mode, 2, 3, SIGSYS,
	"SECCOMP_MODE_FILTER doesn't permit exit()"},

	{check_filter_mode, 0, 4, SIGSYS,
	"SECCOMP_MODE_FILTER doesn't permit exit()"}
};


static int mode_filter_not_supported;

static void check_filter_mode_inherit(void)
{
	int childpid;
	int childstatus;

	childpid = SAFE_FORK();
	if (childpid == 0) {
		tst_res(TPASS, "SECCOMP_MODE_FILTER permits fork(2)");
		exit(0);
	}

	wait4(childpid, &childstatus, 0, NULL);
	if (WIFSIGNALED(childstatus) && WTERMSIG(childstatus) == SIGSYS)
		tst_res(TPASS,
			"SECCOMP_MODE_FILTER has been inherited by child");
	else
		tst_res(TFAIL,
			"SECCOMP_MODE_FILTER isn't been inherited by child");
}

static void check_strict_mode(int val)
{
	int fd;
	char buf[2];

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0666);

	TEST(prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO,
			"prctl(PR_SET_SECCOMP) sets SECCOMP_MODE_STRICT failed");
		return;
	}

	switch (val) {
	case 1:
		tst_res(TPASS,
			"prctl(PR_SET_SECCOMP) sets SECCOMP_MODE_STRICT succeed");
		prctl(PR_GET_SECCOMP);
		tst_res(TFAIL, "prctl(PR_GET_SECCOMP) succeed unexpectedly");
	break;
	case 2:
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "a", 1);
		SAFE_READ(0, fd, buf, 1);
		tst_res(TPASS,
			"SECCOMP_MODE_STRICT permits read(2) write(2) and _exit(2)");
	break;
	case 3:
		close(fd);
		tst_res(TFAIL,
			"SECCOMP_MODE_STRICT permits close(2) unexpectdly");
	break;
	}

	tst_syscall(__NR_exit, 0);
}

static void check_filter_mode(int val)
{
	int fd;

	if (mode_filter_not_supported == 1) {
		tst_res(TCONF, "kernel doesn't support SECCOMP_MODE_FILTER");
		return;
	}

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0666);

	TEST(prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &strict));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO,
			"prctl(PR_SET_SECCOMP) sets SECCOMP_MODE_FILTER failed");
		return;
	}

	switch (val) {
	case 1:
		tst_res(TPASS,
			"prctl(PR_SET_SECCOMP) sets SECCOMP_MODE_FILTER succeed");
		prctl(PR_GET_SECCOMP);
		tst_res(TFAIL, "prctl(PR_GET_SECCOMP) succeed unexpectedly");
	break;
	case 2:
		close(fd);
		tst_res(TPASS, "SECCOMP_MODE_FILTER permits close(2)");
	break;
	case 3:
		exit(0);
	break;
	case 4:
		check_filter_mode_inherit();
	break;
	}

	tst_syscall(__NR_exit, 0);
}

static void verify_prctl(unsigned int n)
{
	int pid;
	int status;
	struct tcase *tc = &tcases[n];

	pid = SAFE_FORK();
	if (pid == 0) {
		tc->func_check(tc->val);
	} else {
		SAFE_WAITPID(pid, &status, 0);
		if (WIFSIGNALED(status) && WTERMSIG(status) == tc->exp_signal) {
			if (tc->pass_flag)
				tst_res(TPASS, "%s", tc->message);
			else
				tst_res(TFAIL, "%s", tc->message);
			return;
		}

		if (tc->pass_flag == 2 && mode_filter_not_supported == 0)
			tst_res(TFAIL,
				"SECCOMP_MODE_FILTER permits exit() unexpectedly");
	}
}

static void setup(void)
{
	TEST(prctl(PR_GET_SECCOMP));
	if (TST_RET == 0) {
		tst_res(TINFO, "kernel supports PR_GET/SET_SECCOMP");

		TEST(prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, NULL));
		if (TST_RET == -1 && TST_ERR == EINVAL) {
			mode_filter_not_supported = 1;
			return;
		}

		tst_res(TINFO, "kernel supports SECCOMP_MODE_FILTER");
		return;
	}

	if (TST_ERR == EINVAL)
		tst_brk(TCONF, "kernel doesn't support PR_GET/SET_SECCOMP");

	tst_brk(TBROK | TTERRNO,
		"current environment doesn't permit PR_GET/SET_SECCOMP");
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_prctl,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
