// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test PR_GET_SECCOMP and PR_SET_SECCOMP with both prctl(2) and seccomp(2).
 * The second one is called via __NR_seccomp using tst_syscall().
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
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/filter.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include "tst_test.h"
#include "tst_kconfig.h"
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

static void check_strict_mode(int mode);
static void check_filter_mode(int mode);

static struct tcase {
	void (*func_check)(int mode);
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

static int strict_not_supported;
static int filter_not_supported;

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

	if (strict_not_supported)
		return;

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0666);

	if (tst_variant == 1) {
		TEST(tst_syscall(__NR_seccomp, SECCOMP_SET_MODE_STRICT, 0, NULL));
		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "seccomp(SECCOMP_SET_MODE_STRICT) error");
	} else {
		TEST(prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT, 0, NULL));

		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "prctl(SECCOMP_MODE_STRICT) error");
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

	if (filter_not_supported)
		return;

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0666);

	if (tst_variant == 1) {
		TEST(tst_syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, 0, &strict));
		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "seccomp(SECCOMP_SET_MODE_FILTER) error");
	} else {
		TEST(prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &strict));

		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "prctl(SECCOMP_MODE_FILTER) error");
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

		if (tc->pass_flag == 2)
			tst_res(TFAIL, "SECCOMP_MODE_FILTER permits exit() unexpectedly");
	}
}

static void setup(void)
{
	static const char * const kconf_strict[] = {"CONFIG_SECCOMP=y", NULL};
	static const char * const kconf_filter[] = {"CONFIG_SECCOMP_FILTER=y", NULL};

	tst_res(TINFO, "Testing variant: %s",
		tst_variant == 1 ? "seccomp()" : "pctrl(PR_SET_SECCOMP)");

	if (tst_kconfig_check(kconf_strict)) {
		tst_brk(TCONF, "kernel doesn't support SECCOMP_MODE_STRICT. "
				"Skipping CONFIG_SECCOMP tests");

		strict_not_supported = 1;
	} else {
		tst_res(TINFO, "kernel supports SECCOMP_MODE_STRICT");
	}

	if (tst_kconfig_check(kconf_filter)) {
		tst_brk(TCONF, "kernel doesn't support SECCOMP_MODE_FILTER. "
				"Skipping CONFIG_SECCOMP_FILTER tests");

		filter_not_supported = 1;
	} else {
		tst_res(TINFO, "kernel supports SECCOMP_MODE_FILTER");
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_prctl,
	.tcnt = ARRAY_SIZE(tcases),
	.test_variants = 2,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
