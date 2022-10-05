/******************************************************************************
 *	Copyright (c) International Business Machines  Corp., 2007
 *	Author: Sharyathi Nagesh <sharyathi@in.ibm.com>
 ******************************************************************************/

/***************************************************************************
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************/

/*
 * DESCRIPTION
 *	check fallocate() with various error conditions that should produce
 *	EBADF, EINVAL and EFBIG.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <endian.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/utsname.h>
#include <limits.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fallocate.h"
#include "lapi/abisize.h"

#define BLOCKS_WRITTEN		12
#ifdef TEST_DEFAULT
# define DEFAULT_TEST_MODE	0
#else
# define DEFAULT_TEST_MODE	1
#endif
#define OFFSET			12
#define FNAMER			"test_file1"
#define FNAMEW			"test_file2"
#define BLOCK_SIZE		1024
#define MAX_FILESIZE            (LLONG_MAX / 1024)

static void setup(void);
static void fallocate_verify(int);
static void cleanup(void);

static int fdw;
static int fdr;

static struct test_data_t {
	int *fd;
	char *fname;
	int mode;
	loff_t offset;
	loff_t len;
	int error;
} test_data[] = {
	{&fdr, FNAMER, DEFAULT_TEST_MODE, 0, 1, EBADF},
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, -1, 1, EINVAL},
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, 1, -1, EINVAL},
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, BLOCKS_WRITTEN, 0, EINVAL},
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, BLOCKS_WRITTEN, -1, EINVAL},
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, -(BLOCKS_WRITTEN+OFFSET), 1, EINVAL},
#if defined(TST_ABI64) || _FILE_OFFSET_BITS == 64
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, MAX_FILESIZE, 1, EFBIG},
	{&fdw, FNAMEW, DEFAULT_TEST_MODE, 1, MAX_FILESIZE, EFBIG},
#endif
};

TCID_DEFINE(fallocate02);
int TST_TOTAL = ARRAY_SIZE(test_data);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			fallocate_verify(i);
	}

	cleanup();

	tst_exit();
}

static void setup(void)
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fdr = SAFE_OPEN(cleanup, FNAMER, O_RDONLY | O_CREAT, S_IRUSR);

	fdw = SAFE_OPEN(cleanup, FNAMEW, O_RDWR | O_CREAT, S_IRWXU);

	char buf[BLOCK_SIZE];
	memset(buf, 'A', BLOCK_SIZE);
	for (i = 0; i < BLOCKS_WRITTEN; i++)
		SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fdw, buf, BLOCK_SIZE);
}

static void fallocate_verify(int i)
{
	TEST(fallocate(*test_data[i].fd, test_data[i].mode,
		       test_data[i].offset * BLOCK_SIZE,
		       test_data[i].len * BLOCK_SIZE));
	if (TEST_ERRNO != test_data[i].error) {
		if (TEST_ERRNO == EOPNOTSUPP ||
		    TEST_ERRNO == ENOSYS) {
			tst_brkm(TCONF, cleanup,
				 "fallocate system call is not implemented");
		}
		tst_resm(TFAIL | TTERRNO,
			 "fallocate(%s:%d, %d, %" PRId64 ", %" PRId64 ") "
			 "failed, expected errno:%d", test_data[i].fname,
			 *test_data[i].fd, test_data[i].mode,
			 test_data[i].offset * BLOCK_SIZE,
			 test_data[i].len * BLOCK_SIZE, test_data[i].error);
	} else {
		tst_resm(TPASS | TTERRNO,
			 "fallocate(%s:%d, %d, %" PRId64 ", %" PRId64 ") "
			 "returned %d", test_data[i].fname, *test_data[i].fd,
			 test_data[i].mode, test_data[i].offset * BLOCK_SIZE,
			 test_data[i].len * BLOCK_SIZE, TEST_ERRNO);
	}
}

static void cleanup(void)
{
	if (fdw > 0)
		SAFE_CLOSE(NULL, fdw);
	if (fdr > 0)
		SAFE_CLOSE(NULL, fdr);

	tst_rmdir();
}
