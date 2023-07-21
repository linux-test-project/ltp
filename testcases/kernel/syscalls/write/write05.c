// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * 07/2001 Ported by John George
 * 04/2002 wjhuie sigset cleanups
 * 08/2007 Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com>
 * Copyright (c) Linux Test Project, 2002-2023
 */

/*\
 * [Description]
 *
 *	Check the return value, and errnos of write(2)
 *
 *	- when the file descriptor is invalid - EBADF
 *	- when the buf parameter is invalid - EFAULT
 *	- on an attempt to write to a pipe that is not open for reading - EPIPE
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "tst_test.h"

static int fd;
static int inv_fd = -1;
static char b[32];
static char *buf = b;
static char *bad_addr;
static int pipefd[2];

static struct tcase {
	int *fd;
	char **buf;
	size_t size;
	int exp_errno;
} tcases[] = {
	{&inv_fd, &buf, sizeof(buf), EBADF},
	{&fd, &bad_addr, sizeof(buf), EFAULT},
	{&pipefd[1], &buf, sizeof(buf), EPIPE},
};

static int sigpipe_cnt;

static void sighandler(int sig)
{
	if (sig == SIGPIPE)
		sigpipe_cnt++;
}

static void verify_write(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	sigpipe_cnt = 0;

	TST_EXP_FAIL2(write(*tc->fd, *tc->buf, tc->size), tc->exp_errno);
	if (TST_RET != -1)
		return;

	if (tc->exp_errno == EPIPE && sigpipe_cnt != 1)
		tst_res(TFAIL, "sigpipe_cnt = %i", sigpipe_cnt);
}

static void setup(void)
{
	fd = SAFE_OPEN("write_test", O_RDWR | O_CREAT, 0644);

	bad_addr = SAFE_MMAP(0, 1, PROT_NONE,
			MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	SAFE_PIPE(pipefd);
	SAFE_CLOSE(pipefd[0]);

	SAFE_SIGNAL(SIGPIPE, sighandler);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	SAFE_MUNMAP(bad_addr, 1);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_write,
	.tcnt = ARRAY_SIZE(tcases),
};
