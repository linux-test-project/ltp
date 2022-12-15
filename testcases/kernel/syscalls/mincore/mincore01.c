/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  Author: Rajeev Tiwari: rajeevti@in.ibm.com
 * Copyright (c) 2004 Gernot Payer <gpayer@suse.de>
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
 * test1:
 *	Invoke mincore() when the start address is not multiple of page size.
 *	EINVAL
 * test2:
 *	Invoke mincore() when the vector points to an invalid address. EFAULT
 * test3:
 *	Invoke mincore() when the starting address + length contained unmapped
 *	memory. ENOMEM
 * test4:
 *      Invoke mincore() when length is greater than (TASK_SIZE - addr). ENOMEM
 *      In Linux 2.6.11 and earlier, the error EINVAL  was  returned for this
 *      condition.
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "safe_macros.h"

static int pagesize;
static rlim_t STACK_LIMIT = 10485760;

static void cleanup(void);
static void setup(void);

char *TCID = "mincore01";

static char *global_pointer;
static unsigned char *global_vec;
static int global_len;

struct test_case_t;
static void setup1(struct test_case_t *tc);
static void setup2(struct test_case_t *tc);
static void setup3(struct test_case_t *tc);
static void setup4(struct test_case_t *tc);

static struct test_case_t {
	char *addr;
	size_t len;
	unsigned char *vector;
	int exp_errno;
	void (*setupfunc) (struct test_case_t *tc);
} TC[] = {
	{NULL, 0, NULL, EINVAL, setup1},
	{NULL, 0, NULL, EFAULT, setup2},
	{NULL, 0, NULL, ENOMEM, setup3},
	{NULL, 0, NULL, ENOMEM, setup4}
};

static void mincore_verify(struct test_case_t *tc);

int TST_TOTAL = ARRAY_SIZE(TC);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			mincore_verify(&TC[i]);
	}

	cleanup();
	tst_exit();
}

static void setup1(struct test_case_t *tc)
{
	tc->addr = global_pointer + 1;
	tc->len = global_len;
	tc->vector = global_vec;
}

void setup2(struct test_case_t *tc)
{
	unsigned char *t;
	struct rlimit limit;

	t = SAFE_MMAP(cleanup, NULL, global_len, PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	SAFE_MUNMAP(cleanup, t, global_len);

	/* set stack limit so that the unmaped pointer is invalid for architectures like s390 */
	limit.rlim_cur = STACK_LIMIT;
	limit.rlim_max = STACK_LIMIT;

	if (setrlimit(RLIMIT_STACK, &limit) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "setrlimit failed");

	tc->addr = global_pointer;
	tc->len = global_len;
	tc->vector = t;
}

static void setup3(struct test_case_t *tc)
{
	tc->addr = global_pointer;
	tc->len = global_len * 2;
	SAFE_MUNMAP(cleanup, global_pointer + global_len, global_len);
	tc->vector = global_vec;
}

static void setup4(struct test_case_t *tc)
{
	struct rlimit as_lim;

	SAFE_GETRLIMIT(cleanup, RLIMIT_AS, &as_lim);

	tc->addr = global_pointer;
	tc->len = as_lim.rlim_cur - (rlim_t)global_pointer + pagesize;
	tc->vector = global_vec;
}

static void setup(void)
{
	char *buf;
	int fd;

	pagesize = getpagesize();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	/* global_pointer will point to a mmapped area of global_len bytes */
	global_len = pagesize * 2;

	buf = SAFE_MALLOC(cleanup, global_len);
	memset(buf, 42, global_len);

	fd = SAFE_OPEN(cleanup, "mincore01", O_CREAT | O_RDWR,
		       S_IRUSR | S_IWUSR);

	SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, buf, global_len);

	free(buf);

	global_pointer = SAFE_MMAP(cleanup, NULL, global_len * 2,
				   PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	global_vec = SAFE_MALLOC(cleanup,
				 (global_len + pagesize - 1) / pagesize);

	SAFE_CLOSE(cleanup, fd);
}

static void mincore_verify(struct test_case_t *tc)
{
	if (tc->setupfunc)
		tc->setupfunc(tc);

	TEST(mincore(tc->addr, tc->len, tc->vector));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"mincore failed unexpectedly; expected: "
			"%d - %s", tc->exp_errno,
			strerror(tc->exp_errno));
	}
}

static void cleanup(void)
{
	free(global_vec);

	if (munmap(global_pointer, global_len) == -1)
		tst_resm(TWARN | TERRNO, "munmap failed");

	tst_rmdir();
}
