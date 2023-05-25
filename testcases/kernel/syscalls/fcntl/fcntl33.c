// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*
 * DESCRIPTION
 *  Test for feature F_SETLEASE of fcntl(2).
 *  "F_SETLEASE is used to establish a lease which provides a mechanism:
 *   When a process (the lease breaker) performs an open(2) or truncate(2)
 *   that conflicts with the lease, the system call will be blocked by
 *   kernel, meanwhile the kernel notifies the lease holder by sending
 *   it a signal (SIGIO by default), after the lease holder successes
 *   to downgrade or remove the lease, the kernel permits the system
 *   call of the lease breaker to proceed."
 */

#include <errno.h>

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_macros.h"

/*
 * MIN_TIME_LIMIT is defined to 5 senconds as a minimal acceptable
 * amount of time for the lease breaker waiting for unblock via
 * lease holder voluntarily downgrade or remove the lease, if the
 * lease breaker is unblocked within MIN_TIME_LIMIT we may consider
 * that the feature of the lease mechanism works well.
 *
 * The lease-break-time is set to 45 seconds for timeout in kernel.
 */
#define MIN_TIME_LIMIT	5

#define OP_OPEN_RDONLY	0
#define OP_OPEN_WRONLY	1
#define OP_OPEN_RDWR	2
#define OP_TRUNCATE	3

#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)
#define PATH_LS_BRK_T	"/proc/sys/fs/lease-break-time"

static void do_test(unsigned int);
static void do_child(unsigned int);

static int fd;
static int ls_brk_t;
static long type;
static sigset_t newset, oldset;

/* Time limit for lease holder to receive SIGIO. */
static struct timespec timeout = {.tv_sec = 5};

static struct test_case_t {
	int lease_type;
	int op_type;
	char *desc;
} test_cases[] = {
	{F_WRLCK, OP_OPEN_RDONLY,
		"open(O_RDONLY) conflicts with fcntl(F_SETLEASE, F_WRLCK)"},
	{F_WRLCK, OP_OPEN_WRONLY,
		"open(O_WRONLY) conflicts with fcntl(F_SETLEASE, F_WRLCK)"},
	{F_WRLCK, OP_OPEN_RDWR,
		"open(O_RDWR) conflicts with fcntl(F_SETLEASE, F_WRLCK)"},
	{F_WRLCK, OP_TRUNCATE,
		"truncate() conflicts with fcntl(F_SETLEASE, F_WRLCK)"},
	{F_RDLCK, OP_OPEN_WRONLY,
		"open(O_WRONLY) conflicts with fcntl(F_SETLEASE, F_RDLCK)"},
	{F_RDLCK, OP_OPEN_RDWR,
		"open(O_RDWR) conflicts with fcntl(F_SETLEASE, F_RDLCK)"},
	{F_RDLCK, OP_TRUNCATE,
		"truncate() conflicts with fcntl(F_SETLEASE, F_RDLCK)"},
};

static void setup(void)
{
	tst_timer_check(CLOCK_MONOTONIC);

	/* Backup and set the lease-break-time. */
	SAFE_FILE_SCANF(PATH_LS_BRK_T, "%d", &ls_brk_t);
	SAFE_FILE_PRINTF(PATH_LS_BRK_T, "%d", 45);

	SAFE_TOUCH("file", FILE_MODE, NULL);

	sigemptyset(&newset);
	sigaddset(&newset, SIGIO);

	if (sigprocmask(SIG_SETMASK, &newset, &oldset) < 0)
		tst_brk(TBROK | TERRNO, "sigprocmask() failed");
}

static void do_test(unsigned int i)
{
	pid_t cpid;

	cpid = SAFE_FORK();
	if (cpid == 0) {
		do_child(i);
		return;
	}

	fd = SAFE_OPEN("file", O_RDONLY);

	TEST(fcntl(fd, F_SETLEASE, test_cases[i].lease_type));
	if (TST_RET == -1) {
		if (type == TST_OVERLAYFS_MAGIC && TST_ERR == EAGAIN) {
			tst_res(TINFO | TTERRNO,
				"fcntl(F_SETLEASE, F_WRLCK) failed on overlayfs as expected");
		} else {
			tst_res(TFAIL | TTERRNO, "fcntl() failed to set lease");
		}
		TST_CHECKPOINT_WAKE(0);
		goto exit;
	}

	TST_CHECKPOINT_WAKE(0);
	/* Wait for SIGIO caused by lease breaker. */
	TEST(sigtimedwait(&newset, NULL, &timeout));
	if (TST_RET == -1) {
		if (TST_ERR == EAGAIN) {
			tst_res(TFAIL | TTERRNO,
				"failed to receive SIGIO within %lis",
				timeout.tv_sec);
			goto exit;
		}
		tst_brk(TBROK | TTERRNO, "sigtimedwait() failed");
	}

	/* Try to downgrade or remove the lease. */
	switch (test_cases[i].lease_type) {
	case F_WRLCK:
		TEST(fcntl(fd, F_SETLEASE, F_RDLCK));
		if (TST_RET == 0) {
			if (test_cases[i].op_type == OP_OPEN_RDONLY)
				break;

			tst_res(TFAIL,
				"fcntl() downgraded lease when not read-only");
		}

		if (test_cases[i].op_type == OP_OPEN_RDONLY) {
			tst_res(TFAIL | TTERRNO,
				"fcntl() failed to downgrade lease");
		}

		/* Falls through */
	case F_RDLCK:
		TEST(fcntl(fd, F_SETLEASE, F_UNLCK));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO,
				 "fcntl() failed to remove the lease");
		}
		break;
	default:
		break;
	}

exit:
	tst_reap_children();
	SAFE_CLOSE(fd);
}

static void do_child(unsigned int i)
{
	long long elapsed_ms;

	TST_CHECKPOINT_WAIT(0);

	tst_timer_start(CLOCK_MONOTONIC);

	switch (test_cases[i].op_type) {
	case OP_OPEN_RDONLY:
		SAFE_OPEN("file", O_RDONLY);
		break;
	case OP_OPEN_WRONLY:
		SAFE_OPEN("file", O_WRONLY);
		break;
	case OP_OPEN_RDWR:
		SAFE_OPEN("file", O_RDWR);
		break;
	case OP_TRUNCATE:
		SAFE_TRUNCATE("file", 0);
		break;
	default:
		break;
	}

	tst_timer_stop();

	elapsed_ms = tst_timer_elapsed_ms();

	if (elapsed_ms < MIN_TIME_LIMIT * 1000) {
		tst_res(TPASS, "%s, unblocked within %ds",
			 test_cases[i].desc, MIN_TIME_LIMIT);
	} else {
		tst_res(TFAIL,
			"%s, blocked too long %llims, expected within %ds",
			test_cases[i].desc, elapsed_ms, MIN_TIME_LIMIT);
	}
}

static void cleanup(void)
{
	if (sigprocmask(SIG_SETMASK, &oldset, NULL) < 0)
		tst_res(TWARN | TERRNO, "sigprocmask restore oldset failed");

	if (fd > 0)
		SAFE_CLOSE(fd);

	/* Restore the lease-break-time. */
	SAFE_FILE_PRINTF(PATH_LS_BRK_T, "%d", ls_brk_t);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.test = do_test,
	.cleanup = cleanup,
	.skip_filesystems = (const char *const []) {
		"tmpfs",
		"ramfs",
		"nfs",
		NULL
	},
};
