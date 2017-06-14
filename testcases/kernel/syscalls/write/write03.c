/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by John George
 *   Copyright (c) 2017 Fujitsu Ltd.
 *	04/2017 Modified by Jinhui Huang
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
 * DESCRIPTION
 *	Testcase to check that write(2) doesn't corrupt a file when it fails
 *
 * ALGORITHM
 *	Create a file for writing, write 100 bytes to it. Then make write(2)
 *	fail with some erroneous parameter, close the fd. Then reopen the
 *	file in RDONLY mode, and read the contents of the file. Compare the
 *	buffers, to see whether they are same.
 */

#include <stdio.h>
#include <errno.h>
#include "tst_test.h"

static char *bad_addr;
static char wbuf[BUFSIZ], rbuf[BUFSIZ];
static int fd;

static void verify_write(void)
{
	fd = SAFE_CREAT("testfile", 0644);

	SAFE_WRITE(1, fd, wbuf, 100);

	if (write(fd, bad_addr, 100) != -1) {
		tst_res(TFAIL, "write() failed to fail");
		SAFE_CLOSE(fd);
		return;
	}

	SAFE_CLOSE(fd);

	fd = SAFE_OPEN("testfile", O_RDONLY);

	memset(rbuf, 0, BUFSIZ);

	SAFE_READ(0, fd, rbuf, 100);

	if (memcmp(wbuf, rbuf, 100) == 0)
		tst_res(TPASS, "failure of write() did not corrupt the file");
	else
		tst_res(TFAIL, "failure of write() corrupted the file");

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	bad_addr = SAFE_MMAP(0, 1, PROT_NONE,
			MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	memset(wbuf, '0', 100);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_write,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
