// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2012
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test errno codes in process_vm_readv and process_vm_writev syscalls.
 */

#include <pwd.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

struct process_vm_params {
	int len;
	char *ldummy;
	char *rdummy;
	pid_t pid;
	struct iovec *lvec;
	unsigned long liovcnt;
	struct iovec *rvec;
	unsigned long riovcnt;
	unsigned long flags;
};

static char *str_read;
static void (*test_params)(struct process_vm_params *params);

static struct process_vm_params *alloc_params(void)
{
	struct process_vm_params *sane_params;
	int len;

	len = getpagesize();

	sane_params = SAFE_MALLOC(sizeof(struct process_vm_params));
	sane_params->len = len;
	sane_params->ldummy = SAFE_MALLOC(len);
	sane_params->rdummy = SAFE_MALLOC(len);

	sane_params->lvec = SAFE_MALLOC(sizeof(struct process_vm_params));
	sane_params->lvec->iov_base = sane_params->ldummy;
	sane_params->lvec->iov_len = len;
	sane_params->liovcnt = 1;

	sane_params->rvec = SAFE_MALLOC(sizeof(struct process_vm_params));
	sane_params->rvec->iov_base = sane_params->rdummy;
	sane_params->rvec->iov_len = len;
	sane_params->riovcnt = 1;

	sane_params->flags = 0;
	sane_params->pid = getpid();

	return sane_params;
}

static void free_params(struct process_vm_params *params)
{
	if (params) {
		free(params->ldummy);
		free(params->rdummy);
		free(params->lvec);
		free(params->rvec);
		free(params);
	}
}

static void test_readv(struct process_vm_params *params)
{
	TEST(tst_syscall(__NR_process_vm_readv,
		params->pid,
		params->lvec, params->liovcnt,
		params->rvec, params->riovcnt,
		params->flags));
}

static void test_writev(struct process_vm_params *params)
{
	TEST(tst_syscall(__NR_process_vm_writev,
		params->pid,
		params->lvec, params->liovcnt,
		params->rvec, params->riovcnt,
		params->flags));
}

static void check_errno(long expected_errno)
{
	if (TST_ERR == expected_errno)
		tst_res(TPASS | TTERRNO, "expected failure");
	else if (TST_ERR == 0)
		tst_res(TFAIL, "call succeeded unexpectedly");
	else
		tst_res(TFAIL | TTERRNO, "unexpected failure - "
			"expected = %ld : %s, actual",
			expected_errno, strerror(expected_errno));
}

static void test_sane_params(void)
{
	struct process_vm_params *sane_params;

	tst_res(TINFO, "Testing sane parameters");

	sane_params = alloc_params();
	test_params(sane_params);
	TST_EXP_EQ_LI(TST_RET, sane_params->len);
	free_params(sane_params);
}

static void test_flags(void)
{
	struct process_vm_params *params;
	long flags[] = { -INT_MAX, -1, 1, INT_MAX, 0 };
	int flags_size = ARRAY_SIZE(flags) / sizeof(flags[0]);
	int i;

	params = alloc_params();

	for (i = 0; i < flags_size; i++) {
		params->flags = flags[i];

		tst_res(TINFO, "Testing flags=%ld", flags[i]);
		test_params(params);

		/* atm. only flags == 0 is allowed, everything else
		 * should fail with EINVAL
		 */
		if (flags[i] != 0) {
			TST_EXP_EQ_LI(TST_RET, -1);
			check_errno(EINVAL);
		} else {
			TST_EXP_EQ_LI(TST_RET, params->len);
		}
	}

	free_params(params);
}

static void test_iov_len_overflow(void)
{
	struct process_vm_params *params;

	tst_res(TINFO, "Testing iov_len = -1");

	params = alloc_params();
	params->lvec->iov_len = -1;
	params->rvec->iov_len = -1;
	test_params(params);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EINVAL);
	free_params(params);
}

static void test_iov_invalid(void)
{
	struct process_vm_params *sane_params;
	struct process_vm_params params_copy;

	sane_params = alloc_params();

	tst_res(TINFO, "Testing lvec->iov_base = -1");
	params_copy = *sane_params;
	params_copy.lvec->iov_base = (void *)-1;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EFAULT);

	tst_res(TINFO, "Testing rvec->iov_base = -1");
	params_copy = *sane_params;
	params_copy.rvec->iov_base = (void *)-1;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EFAULT);

	tst_res(TINFO, "Testing lvec = -1");
	params_copy = *sane_params;
	params_copy.lvec = (void *)-1;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EFAULT);

	tst_res(TINFO, "Testing rvec = -1");
	params_copy = *sane_params;
	params_copy.rvec = (void *)-1;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EFAULT);

	free_params(sane_params);
}

static void test_invalid_pid(void)
{
	pid_t invalid_pid = -1;
	struct process_vm_params *params;
	struct process_vm_params params_copy;

	params = alloc_params();

	tst_res(TINFO, "Testing invalid PID");
	params_copy = *params;
	params_copy.pid = invalid_pid;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(ESRCH);

	tst_res(TINFO, "Testing unused PID");
	params_copy = *params;
	invalid_pid = tst_get_unused_pid();
	params_copy.pid = invalid_pid;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(ESRCH);

	free_params(params);
}

static void test_invalid_perm(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;
	struct process_vm_params *params;
	pid_t child_pid;
	pid_t parent_pid;

	tst_res(TINFO, "Testing invalid permissions on given PID");

	parent_pid = getpid();
	child_pid = SAFE_FORK();
	if (!child_pid) {
		ltpuser = SAFE_GETPWNAM(nobody_uid);
		SAFE_SETUID(ltpuser->pw_uid);

		params = alloc_params();
		params->pid = parent_pid;
		test_params(params);
		TST_EXP_EQ_LI(TST_RET, -1);
		check_errno(EPERM);
		free_params(params);
		return;
	}

	/* collect result from child  before the next test, otherwise
	 * TFAIL/TPASS messages will arrive asynchronously
	 */
	tst_reap_children();
}

static void test_invalid_protection(void)
{
	struct process_vm_params *sane_params;
	struct process_vm_params params_copy;
	void *data;
	int len;

	len = getpagesize();
	sane_params = alloc_params();
	data = SAFE_MMAP(NULL, len, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	tst_res(TINFO, "Testing data with invalid protection (lvec)");
	params_copy = *sane_params;
	params_copy.lvec->iov_base = data;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EFAULT);

	tst_res(TINFO, "Testing data with invalid protection (rvec)");
	params_copy = *sane_params;
	params_copy.rvec->iov_base = data;
	test_params(&params_copy);
	TST_EXP_EQ_LI(TST_RET, -1);
	check_errno(EFAULT);

	SAFE_MUNMAP(data, len);
	free_params(sane_params);
}

static void run(void)
{
	test_sane_params();
	test_flags();
	test_iov_len_overflow();
	test_iov_invalid();
	test_invalid_pid();
	test_invalid_perm();
	test_invalid_protection();
}

static void setup(void)
{
	if (str_read) {
		tst_res(TINFO, "Selected process_vm_readv");
		test_params = test_readv;
	} else {
		tst_res(TINFO, "Selected process_vm_writev");
		test_params = test_writev;
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"r", &str_read, "Use process_vm_read instead of process_vm_write"},
		{},
	},
};
