/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 * Verify that,
 *  1. msync() fails with -1 return value and sets errno to EBUSY
 *     if MS_INVALIDATE was specified in flags, and a memory lock
 *     exists for the specified address range.
 *  2. msync() fails with -1 return value and sets errno to EINVAL
 *     if addr is not a multiple of PAGESIZE; or any bit other than
 *     MS_ASYNC | MS_INVALIDATE | MS_SYNC is set in flags; or both
 *     MS_SYNC and MS_ASYNC are set in flags.
 *  3. msync() fails with -1 return value and sets errno to ENOMEM
 *     if the indicated memory (or part of it) was not mapped.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <pwd.h>
#include <sys/resource.h>

#include "test.h"
#include "safe_macros.h"

#define INV_SYNC	-1
#define TEMPFILE	"msync_file"
#define BUF_SIZE	256

static void setup(void);
static void cleanup(void);

static int fd;
static char *addr1;
static char *addr2;
static char *addr3;
static char *addr4;

static size_t page_sz;

static struct test_case_t {
	char **addr;
	int flags;
	int exp_errno;
} test_cases[] = {
	{ &addr1, MS_INVALIDATE, EBUSY },
	{ &addr1, MS_ASYNC | MS_SYNC, EINVAL },
	{ &addr1, INV_SYNC, EINVAL },
	{ &addr2, MS_SYNC, EINVAL },
	{ &addr3, MS_SYNC, EINVAL },
	{ &addr4, MS_SYNC, ENOMEM },
};

static void msync_verify(struct test_case_t *tc);

char *TCID = "msync03";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			msync_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	size_t nwrite = 0;
	char write_buf[BUF_SIZE];
	struct rlimit rl;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	page_sz = (size_t)sysconf(_SC_PAGESIZE);

	fd = SAFE_OPEN(cleanup, TEMPFILE, O_RDWR | O_CREAT, 0666);

	memset(write_buf, 'a', BUF_SIZE);
	while (nwrite < page_sz) {
		SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, write_buf, BUF_SIZE);
		nwrite += BUF_SIZE;
	}

	addr1 = SAFE_MMAP(cleanup, 0, page_sz, PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_LOCKED, fd, 0);

	/* addr2 is not a multiple of PAGESIZE */
	addr2 = addr1 + 1;

	/* addr3 is outside the address space of the process */
	SAFE_GETRLIMIT(NULL, RLIMIT_DATA, &rl);
	addr3 = (char *)rl.rlim_max;

	/* memory pointed to by addr4 was not mapped */
	addr4 = sbrk(0) + (4 * page_sz);
}

static void msync_verify(struct test_case_t *tc)
{
	TEST(msync(*(tc->addr), page_sz, tc->flags));
	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "msync succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "msync failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "msync failed unexpectedly; expected: "
			 "%d - %s", tc->exp_errno,
			 strerror(tc->exp_errno));
	}
}

static void cleanup(void)
{
	if (addr1 && munmap(addr1, page_sz) < 0)
		tst_resm(TWARN | TERRNO, "munmap() failed");

	if (fd > 0 && close(fd) < 0)
		tst_resm(TWARN | TERRNO, "close() failed");

	tst_rmdir();
}
