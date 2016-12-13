/*
 * Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 *
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	Testcase to check the basic functionality of the readv(2) system call.
 *
 * ALGORITHM
 *	Create a IO vector, and attempt to readv() various components of it.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

#define	CHUNK		64

char *TCID = "readv01";
int TST_TOTAL = 1;

static char buf[CHUNK];

static struct iovec rd_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
	{NULL, 0},
};

static int fd;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc, i, fail;
	char *vec;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		SAFE_LSEEK(cleanup, fd, 0, SEEK_SET);

		if (readv(fd, rd_iovec, 0) == -1)
			tst_resm(TFAIL | TERRNO, "readv failed unexpectedly");
		else
			tst_resm(TPASS, "readv read 0 io vectors");

		memset(rd_iovec[0].iov_base, 0x00, CHUNK);

		if (readv(fd, rd_iovec, 3) != CHUNK) {
			tst_resm(TFAIL, "readv failed reading %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		} else {
			fail = 0;
			vec = rd_iovec[0].iov_base;

			for (i = 0; i < CHUNK; i++) {
				if (vec[i] != 0x42)
					fail++;
			}

			if (fail)
				tst_resm(TFAIL, "Wrong buffer content");
			else
				tst_resm(TPASS, "readv passed reading %d bytes "
				         "followed by two NULL vectors", CHUNK);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	memset(buf, 0x42, sizeof(buf));

	fd = SAFE_OPEN(cleanup, "data_file", O_WRONLY | O_CREAT, 0666);
	SAFE_WRITE(cleanup, 1, fd, buf, sizeof(buf));
	SAFE_CLOSE(cleanup, fd);
	fd = SAFE_OPEN(cleanup, "data_file", O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	tst_rmdir();
}
