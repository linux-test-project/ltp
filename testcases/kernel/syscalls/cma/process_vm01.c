/*
 * Copyright (C) 2012 Linux Test Project, Inc.
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
 * errno tests shared by process_vm_readv, process_vm_writev tests.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include "config.h"
#include "test.h"
#include "safe_macros.h"
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

static int rflag;
static int wflag;

static option_t options[] = {
	{"r", &rflag, NULL},
	{"w", &wflag, NULL},
	{NULL, NULL, NULL}
};

static char TCID_readv[] = "process_vm_readv";
static char TCID_writev[] = "process_vm_writev";
char *TCID = "cma01";
int TST_TOTAL = 1;
static void (*cma_test_params) (struct process_vm_params * params) = NULL;

static void setup(char *argv[]);
static void cleanup(void);
static void help(void);

static void cma_test_params_read(struct process_vm_params *params);
static void cma_test_params_write(struct process_vm_params *params);
static void cma_test_errnos(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, &help);

	setup(argv);
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		cma_test_errnos();
	}
	cleanup();
	tst_exit();
}

static void setup(char *argv[])
{
	tst_require_root();

	if (rflag && wflag)
		tst_brkm(TBROK, NULL, "Parameters -r -w can not be used"
			 " at the same time.");
	else if (rflag) {
		TCID = TCID_readv;
		cma_test_params = cma_test_params_read;
	} else if (wflag) {
		TCID = TCID_writev;
		cma_test_params = cma_test_params_write;
	} else
		tst_brkm(TBROK, NULL, "Parameter missing, required -r or -w.");
	TEST_PAUSE;
}

static void cleanup(void)
{
}

static void help(void)
{
	printf("  -r      Use process_vm_readv\n");
	printf("  -w      Use process_vm_writev\n");
}

static void cma_test_params_read(struct process_vm_params *params)
{
	TEST(ltp_syscall(__NR_process_vm_readv,
			 params->pid,
			 params->lvec, params->liovcnt,
			 params->rvec, params->riovcnt,
			 params->flags));
}

static void cma_test_params_write(struct process_vm_params *params)
{
	TEST(ltp_syscall(__NR_process_vm_writev,
			 params->pid,
			 params->lvec, params->liovcnt,
			 params->rvec, params->riovcnt,
			 params->flags));
}

static int cma_check_ret(long expected_ret, long act_ret)
{
	if (expected_ret == act_ret) {
		tst_resm(TPASS, "expected ret success - "
			 "returned value = %ld", act_ret);
	} else {
		tst_resm(TFAIL, "unexpected failure - "
			 "returned value = %ld, expected: %ld",
			 act_ret, expected_ret);
		return 1;
	}
	return 0;
}

static int cma_check_errno(long expected_errno)
{
	if (TEST_ERRNO == expected_errno)
		tst_resm(TPASS | TTERRNO, "expected failure");
	else if (TEST_ERRNO == 0) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
		return 1;
	} else {
		tst_resm(TFAIL | TTERRNO, "unexpected failure - "
			 "expected = %ld : %s, actual",
			 expected_errno, strerror(expected_errno));
		return 2;
	}
	return 0;
}

static struct process_vm_params *cma_alloc_sane_params(void)
{
	struct process_vm_params *sane_params;
	int len;

	len = getpagesize();
	sane_params = SAFE_MALLOC(NULL, sizeof(struct process_vm_params));
	sane_params->len = len;
	sane_params->ldummy = SAFE_MALLOC(NULL, len);
	sane_params->rdummy = SAFE_MALLOC(NULL, len);

	sane_params->lvec = SAFE_MALLOC(NULL, sizeof(struct iovec));
	sane_params->lvec->iov_base = sane_params->ldummy;
	sane_params->lvec->iov_len = len;
	sane_params->liovcnt = 1;

	sane_params->rvec = SAFE_MALLOC(NULL, sizeof(struct iovec));
	sane_params->rvec->iov_base = sane_params->rdummy;
	sane_params->rvec->iov_len = len;
	sane_params->riovcnt = 1;

	sane_params->flags = 0;
	sane_params->pid = getpid();

	return sane_params;
}

static void cma_free_params(struct process_vm_params *params)
{
	if (params) {
		free(params->ldummy);
		free(params->rdummy);
		free(params->lvec);
		free(params->rvec);
		free(params);
	}
}

static void cma_test_sane_params(void)
{
	struct process_vm_params *sane_params;

	sane_params = cma_alloc_sane_params();
	tst_resm(TINFO, "test_sane_params");
	cma_test_params(sane_params);
	cma_check_ret(sane_params->len, TEST_RETURN);
	cma_free_params(sane_params);
}

static void cma_test_flags(void)
{
	struct process_vm_params *params;
	long flags[] = { -INT_MAX, -1, 1, INT_MAX, 0 };
	int flags_size = sizeof(flags) / sizeof(flags[0]);
	int i;

	params = cma_alloc_sane_params();
	for (i = 0; i < flags_size; i++) {
		params->flags = flags[i];
		tst_resm(TINFO, "test_flags, flags=%ld", flags[i]);
		cma_test_params(params);
		/* atm. only flags == 0 is allowed, everything else
		 * should fail with EINVAL */
		if (flags[i] != 0) {
			cma_check_ret(-1, TEST_RETURN);
			cma_check_errno(EINVAL);
		} else {
			cma_check_ret(params->len, TEST_RETURN);
		}
	}
	cma_free_params(params);
}

static void cma_test_iov_len_overflow(void)
{
	struct process_vm_params *params;
	ssize_t maxlen = -1;
	params = cma_alloc_sane_params();

	params->lvec->iov_len = maxlen;
	params->rvec->iov_len = maxlen;
	tst_resm(TINFO, "test_iov_len_overflow");
	cma_test_params(params);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EINVAL);
	cma_free_params(params);
}

static void cma_test_iov_invalid(void)
{
	struct process_vm_params *sane_params;
	struct process_vm_params params_copy;

	sane_params = cma_alloc_sane_params();
	/* make a shallow copy we can 'damage' */

	params_copy = *sane_params;
	tst_resm(TINFO, "test_iov_invalid - lvec->iov_base");
	params_copy.lvec->iov_base = (void *)-1;
	cma_test_params(&params_copy);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EFAULT);

	params_copy = *sane_params;
	tst_resm(TINFO, "test_iov_invalid - rvec->iov_base");
	params_copy.rvec->iov_base = (void *)-1;
	cma_test_params(&params_copy);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EFAULT);

	params_copy = *sane_params;
	tst_resm(TINFO, "test_iov_invalid - lvec");
	params_copy.lvec = (void *)-1;
	cma_test_params(&params_copy);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EFAULT);

	params_copy = *sane_params;
	tst_resm(TINFO, "test_iov_invalid - rvec");
	params_copy.rvec = (void *)-1;
	cma_test_params(&params_copy);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EFAULT);

	cma_free_params(sane_params);
}

static void cma_test_invalid_pid(void)
{
	pid_t invalid_pid = -1;
	struct process_vm_params *params;

	params = cma_alloc_sane_params();
	tst_resm(TINFO, "test_invalid_pid");
	params->pid = invalid_pid;
	cma_test_params(params);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(ESRCH);
	cma_free_params(params);

	invalid_pid = tst_get_unused_pid(cleanup);

	params = cma_alloc_sane_params();
	params->pid = invalid_pid;
	cma_test_params(params);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(ESRCH);
	cma_free_params(params);
}

static void cma_test_invalid_perm(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;
	int status;
	struct process_vm_params *params;
	pid_t child_pid;
	pid_t parent_pid;
	int ret = 0;

	tst_resm(TINFO, "test_invalid_perm");
	parent_pid = getpid();
	child_pid = fork();
	switch (child_pid) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
		break;
	case 0:
		ltpuser = getpwnam(nobody_uid);
		if (ltpuser == NULL)
			tst_brkm(TBROK | TERRNO, NULL, "getpwnam failed");
		SAFE_SETUID(NULL, ltpuser->pw_uid);

		params = cma_alloc_sane_params();
		params->pid = parent_pid;
		cma_test_params(params);
		ret |= cma_check_ret(-1, TEST_RETURN);
		ret |= cma_check_errno(EPERM);
		cma_free_params(params);
		exit(ret);
	default:
		SAFE_WAITPID(cleanup, child_pid, &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child returns %d", status);
	}
}

static void cma_test_invalid_protection(void)
{
	struct process_vm_params *sane_params;
	struct process_vm_params params_copy;
	void *p;

	sane_params = cma_alloc_sane_params();
	/* make a shallow copy we can 'damage' */

	p = mmap(NULL, getpagesize(), PROT_NONE,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (p == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap");

	params_copy = *sane_params;
	params_copy.lvec->iov_base = p;
	tst_resm(TINFO, "test_invalid_protection lvec");
	cma_test_params(&params_copy);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EFAULT);

	params_copy = *sane_params;
	params_copy.rvec->iov_base = p;
	tst_resm(TINFO, "test_invalid_protection rvec");
	cma_test_params(&params_copy);
	cma_check_ret(-1, TEST_RETURN);
	cma_check_errno(EFAULT);

	SAFE_MUNMAP(cleanup, p, getpagesize());

	cma_free_params(sane_params);
}

static void cma_test_errnos(void)
{
	cma_test_sane_params();
	cma_test_flags();
	cma_test_iov_len_overflow();
	cma_test_iov_invalid();
	cma_test_invalid_pid();
	cma_test_invalid_perm();
	cma_test_invalid_protection();
}
