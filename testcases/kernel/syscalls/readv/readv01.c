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
#include <sys/fcntl.h>
#include <memory.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

#define	K_1	1024

#define	NBUFS		4
#define	CHUNK		64
#define	MAX_IOVEC	16

char *TCID = "readv01";
int TST_TOTAL = 1;

static char buf1[K_1], buf2[K_1], buf3[K_1];

static struct iovec rd_iovec[MAX_IOVEC] = {
	/* iov_base *//* iov_len */

	/* Test case #1 */
	{(buf2 + CHUNK * 10), CHUNK},

	/* Test case #2 */
	{(buf2 + CHUNK * 11), CHUNK}
};

static int fd;
static char *buf_list[NBUFS];

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		SAFE_LSEEK(cleanup, fd, CHUNK * 11, SEEK_SET);

		if (readv(fd, (rd_iovec + 0), 0) == -1) {
			tst_resm(TFAIL, "readv() failed with unexpected errno "
				 "%d", errno);
		} else {
			tst_resm(TPASS, "readv read 0 io vectors");
		}

		SAFE_LSEEK(cleanup, fd, CHUNK * 12, SEEK_SET);

		if (readv(fd, (rd_iovec + 1), 4) != CHUNK) {
			tst_resm(TFAIL, "readv failed reading %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		} else {
			tst_resm(TPASS, "readv passed reading %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		}
	}

	cleanup();
	tst_exit();
}

static int fill_mem(char *c_ptr, int c1, int c2)
{
	int count;

	for (count = 1; count <= K_1 / CHUNK; count++) {
		if (count & 0x01) {	/* if odd */
			memset(c_ptr, c1, CHUNK);
		} else {	/* if even */
			memset(c_ptr, c2, CHUNK);
		}
	}
	return 0;
}

static int init_buffs(char *pbufs[])
{
	int i;

	for (i = 0; pbufs[i] != NULL; i++) {
		switch (i) {
		case 0:
		 /*FALLTHROUGH*/ case 1:
			fill_mem(pbufs[i], 0, 1);
			break;

		case 2:
			fill_mem(pbufs[i], 1, 0);
			break;

		default:
			tst_brkm(TBROK, cleanup, "Error in init_buffs()");
		}
	}
	return 0;
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	buf_list[0] = buf1;
	buf_list[1] = buf2;
	buf_list[2] = buf3;
	buf_list[3] = NULL;

	init_buffs(buf_list);

	fd = SAFE_OPEN(cleanup, "data_file", O_WRONLY | O_CREAT, 0666);
	SAFE_WRITE(cleanup, 1, fd, buf_list[2], K_1);
	SAFE_CLOSE(cleanup, fd);
	fd = SAFE_OPEN(cleanup, "data_file", O_RDONLY);
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (fd > 0)
		close(fd);

	tst_rmdir();
}
