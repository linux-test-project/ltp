// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2020 Petr Vorel <petr.vorel@gmail.com>
 * Copyright (c) Linux Test Project, 2002-2023
 * 07/2001 Ported by Wayne Boyer
 * 04/2002 Fixes by wjhuie
 */

/*\
 * [Description]
 *
 * Testcase to check the errnos set by the ioctl(2) system call.
 *
 * - EBADF: Pass an invalid fd to ioctl(fd, ...) and expect EBADF
 * - EFAULT: Pass an invalid address of arg in ioctl(fd, ..., arg)
 * - EINVAL: Pass invalid cmd in ioctl(fd, cmd, arg)
 * - ENOTTY: Pass an non-streams fd in ioctl(fd, cmd, arg)
 * - EFAULT: Pass a NULL address for termio
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <pty.h>
#include "tst_test.h"
#include "lapi/ioctl.h"

#define	INVAL_IOCTL	9999999

static int amaster, aslave;
static int fd, fd_file;
static int bfd = -1;

static struct termio termio;
static struct termios termios;

static struct tcase {
	const char *desc;
	int *fd;
	int request;
	void *s_tio;
	int error;
} tcases[] = {
	{"File descriptor is invalid (termio)", &bfd, TCGETA, &termio, EBADF},
	{"File descriptor is invalid (termios)", &bfd, TCGETS, &termios, EBADF},
	{"Termio address is invalid", &fd, TCGETA, (struct termio *)-1, EFAULT},
	{"Termios address is invalid", &fd, TCGETS, (struct termios *)-1, EFAULT},
	/* This errno value was changed from EINVAL to ENOTTY
	 * by kernel commit 07d106d0 and bbb63c51
	 */
	{"Command is invalid", &fd, INVAL_IOCTL, &termio, ENOTTY},
	{"File descriptor is for a regular file (termio)", &fd_file, TCGETA, &termio, ENOTTY},
	{"File descriptor is for a regular file (termios)", &fd_file, TCGETS, &termios, ENOTTY},
	{"Termio is NULL", &fd, TCGETA, NULL, EFAULT},
	{"Termios is NULL", &fd, TCGETS, NULL, EFAULT}
};

static void verify_ioctl(unsigned int i)
{
	TST_EXP_FAIL(ioctl(*(tcases[i].fd), tcases[i].request, tcases[i].s_tio),
		     tcases[i].error, "%s", tcases[i].desc);
}

static void setup(void)
{
	if (openpty(&amaster, &aslave, NULL, NULL, NULL) < 0)
		tst_brk(TBROK | TERRNO, "unable to open pty");

	fd = amaster;
	fd_file = SAFE_OPEN("x", O_CREAT, 0777);
}

static void cleanup(void)
{
	if (amaster > 0)
		SAFE_CLOSE(amaster);
	if (aslave > 0)
		SAFE_CLOSE(aslave);
	if (fd_file > 0)
		SAFE_CLOSE(fd_file);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_ioctl,
	.tcnt = ARRAY_SIZE(tcases)
};
