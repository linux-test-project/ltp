// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*\
 * Tests basic error handling of the pidfd_send_signal
 * system call.
 *
 * - EINVAL Pass invalid flag value to syscall (value chosen
 *   to be unlikely to collide with future extensions)
 * - EBADF Pass a file descriptor that is corresponding to a
 *   regular file instead of a pid directory
 * - EINVAL Pass a signal that is different from the one used
 *   to initialize the siginfo_t struct
 * - EPERM Try to send signal to other process (init) with
 *   missing privileges
 */

#define _GNU_SOURCE
#include <pwd.h>
#include <signal.h>
#include "tst_test.h"
#include "lapi/pidfd.h"
#include "tst_safe_pthread.h"

#define CORRECT_SIGNAL		SIGUSR1
#define DIFFERENT_SIGNAL	SIGUSR2

static siginfo_t info;
static int pidfd;
static int init_pidfd;
static int dummyfd;

static struct tcase {
	int		*fd;
	siginfo_t	*siginf;
	int		signal;
	int		flags;
	int		exp_err;
} tcases[] = {
	{&pidfd, &info, CORRECT_SIGNAL, 99999, EINVAL},
	{&dummyfd, &info, CORRECT_SIGNAL, 0, EBADF},
	{&pidfd, &info, DIFFERENT_SIGNAL, 0, EINVAL},
	{&init_pidfd, &info, CORRECT_SIGNAL, 0, EPERM},
};

static void verify_pidfd_send_signal(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(pidfd_send_signal(*tc->fd, tc->signal, tc->siginf, tc->flags));
	if (tc->exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO,
			"pidfd_send_signal() did not fail with %s but",
			tst_strerrno(tc->exp_err));
		return;
	}

	tst_res(TPASS | TTERRNO,
		"pidfd_send_signal() failed as expected");
}

static void setup(void)
{
	struct passwd *pw;

	pidfd_send_signal_supported();

	pidfd = SAFE_OPEN("/proc/self", O_DIRECTORY | O_CLOEXEC);
	init_pidfd = SAFE_OPEN("/proc/1", O_DIRECTORY | O_CLOEXEC);
	dummyfd = SAFE_OPEN("dummy_file", O_RDWR | O_CREAT, 0664);

	if (getuid() == 0) {
		pw = SAFE_GETPWNAM("nobody");
		SAFE_SETUID(pw->pw_uid);
	}

	memset(&info, 0, sizeof(info));
	info.si_signo = CORRECT_SIGNAL;
	info.si_code = SI_QUEUE;
	info.si_pid = getpid();
	info.si_uid = getuid();
}

static void cleanup(void)
{
	if (dummyfd > 0)
		SAFE_CLOSE(dummyfd);
	if (init_pidfd > 0)
		SAFE_CLOSE(init_pidfd);
	if (pidfd > 0)
		SAFE_CLOSE(pidfd);
}

static struct tst_test test = {
	.test = verify_pidfd_send_signal,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
