// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2020 Petr Vorel <petr.vorel@gmail.com>
 * 07/2001 Ported by Wayne Boyer
 * 04/2002 Fixes by wjhuie
 *
 *	Testcase to check the errnos set by the ioctl(2) system call.
 *
 * ALGORITHM
 *	1. EBADF: Pass an invalid fd to ioctl(fd, ..) and expect EBADF.
 *	2. EFAULT: Pass an invalid address of arg in ioctl(fd, .., arg)
 *	3. EINVAL: Pass invalid cmd in ioctl(fd, cmd, arg)
 *	4. ENOTTY: Pass an non-streams fd in ioctl(fd, cmd, arg)
 *	5. EFAULT: Pass a NULL address for termio
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include "tst_test.h"
#include "lapi/ioctl.h"

#define	INVAL_IOCTL	9999999

static int fd, fd_file;
static int bfd = -1;

static struct termio termio;

static struct tcase {
	int *fd;
	int request;
	struct termio *s_tio;
	int error;
} tcases[] = {
	/* file descriptor is invalid */
	{&bfd, TCGETA, &termio, EBADF},
	/* termio address is invalid */
	{&fd, TCGETA, (struct termio *)-1, EFAULT},
	/* command is invalid */
	/* This errno value was changed from EINVAL to ENOTTY
	 * by kernel commit 07d106d0 and bbb63c51
	 */
	{&fd, INVAL_IOCTL, &termio, ENOTTY},
	/* file descriptor is for a regular file */
	{&fd_file, TCGETA, &termio, ENOTTY},
	/* termio is NULL */
	{&fd, TCGETA, NULL, EFAULT}
};

static char *device;

static void verify_ioctl(unsigned int i)
{
	TEST(ioctl(*(tcases[i].fd), tcases[i].request, tcases[i].s_tio));

	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TST_ERR != tcases[i].error) {
		tst_res(TFAIL | TTERRNO,
			"failed unexpectedly; expected %s",
		        tst_strerrno(tcases[i].error));
		return;
	}

	tst_res(TPASS | TTERRNO, "failed as expected");
}

static void setup(void)
{
	unsigned int i;

	if (!device)
		tst_brk(TBROK, "You must specify a tty device with -D option");

	fd = SAFE_OPEN(device, O_RDWR, 0777);

	if (tst_kvercmp(3, 7, 0) < 0) {
		for (i = 0; i < ARRAY_SIZE(tcases); i++) {
			if (tcases[i].request == INVAL_IOCTL)
				tcases[i].error = EINVAL;
		}
	}

	fd_file = SAFE_OPEN("x", O_CREAT, 0777);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (fd_file > 0)
		SAFE_CLOSE(fd_file);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_ioctl,
	.tcnt = ARRAY_SIZE(tcases),
	.options = (struct tst_option[]) {
		{"D:", &device, "Tty device. For example, /dev/tty[0-9]"},
		{}
	}
};
