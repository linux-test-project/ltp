/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	write05.c
 *
 * DESCRIPTION
 *	Check the return value, and errnos of write(2)
 *	- when the file descriptor is invalid - EBADF
 *	- when the buf parameter is invalid - EFAULT
 *	- on an attempt to write to a pipe that is not open for reading - EPIPE
 *
 * ALGORITHM
 *	Attempt to write on a file with negative file descriptor, check for -1
 *	Attempt to write on a file with invalid buffer, check for -1
 *	Open a pipe and close the read end, attempt to write to the write
 *	end, check for -1.
 *
 * USAGE:  <for command-line>
 *      write05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *      08/2007 Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com>
 *		- Closing the fd before removing the file
 *
 * Restrictions
 *	None
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

char filename[100];
int fd;

char *bad_addr = 0;

static void verify_write(void)
{
	char pbuf[BUFSIZ];
	int pipefildes[2];
	int status, pid;

//block1:
	tst_res(TINFO, "Enter Block 1: test with bad fd");
	if (write(-1, pbuf, 1) != -1) {
		tst_res(TFAIL, "write of invalid fd passed");
	} else {
		if (errno != EBADF) {
			tst_res(TFAIL, "expected EBADF got %d", errno);
		}
		tst_res(TPASS, "received EBADF as expected.");
	}
	tst_res(TINFO, "Exit Block 1");

//block2:
	tst_res(TINFO, "Enter Block 2: test with a bad address");
	fd = creat(filename, 0644);
	if (fd < 0)
		tst_res(TFAIL, "creating a new file failed");

//block2.1:
	if (write(fd, NULL, 0) != 0) {
		tst_res(TFAIL, "write() with nsize 0 in a "
				"valid fd failed with errno: %d, "
				"but should have succeeded", errno);
	} else {
		tst_res(TPASS, "succeeded as expected with nsize = 0");
	}
//block2.2"
	if (write(fd, bad_addr, 10) != -1) {
		tst_res(TFAIL, "write() on an invalid buffer "
			"succeeded, but should have failed");
	} else {
		if (errno != EFAULT) {
			tst_res(TFAIL, "write() returned illegal "
				 "errno: expected EFAULT, got %d",
				 errno);
		}
		tst_res(TPASS, "received EFAULT as expected.");
	}
	tst_res(TINFO, "Exit Block 2");

//block3:
	tst_res(TINFO, "Enter Block 3: test with invalid pipe");
	if ((pid = fork()) == 0) {	/* child */
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
			tst_res(TINFO, "signal failed");
		}
		if (pipe(pipefildes) == -1) {
			tst_brk(TBROK | TERRNO, "can't open pipe");
			exit(errno);
		}
		close(pipefildes[0]);
		if (write(pipefildes[1], pbuf, 1) != -1) {
			tst_res(TFAIL, "write on read-closed pipe succeeded");
			exit(-1);
		} else {
			if (errno != EPIPE) {
				tst_res(TFAIL, "write() failed to set"
					" errno to EPIPE, got: %d",
					errno);
				exit(errno);
			}
			exit(0);
		}
	} else {
		if (pid < 0) {
			tst_res(TFAIL, "Fork failed");
		}
		wait(&status);
		if (WIFSIGNALED(status) &&
			WTERMSIG(status) == SIGPIPE) {
			tst_res(TFAIL, "child set SIGPIPE in exit");
			} else if (WEXITSTATUS(status) != 0) {
				tst_res(TFAIL, "exit status from child "
					 "expected 0 got %d", status >> 8);
			} else {
				tst_res(TPASS, "received EPIPE as expected.");
			}
		tst_res(TINFO, "Exit Block 3");
	}
	SAFE_CLOSE(fd);
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
static void setup(void)
{
	sprintf(filename, "write05.%d", getpid());

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		printf("mmap failed\n");
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
static void cleanup(void)
{
	/* Close the file descriptor befor removing the file */
	if (fd > 0)
		SAFE_CLOSE(fd);

	unlink(filename);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
};
