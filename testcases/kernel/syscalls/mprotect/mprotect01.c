/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 *	Testcase to check the error conditions for mprotect(2)
 *
 * ALGORITHM
 *	test1:
 *		Invoke mprotect() with an address of 0. Check if error
 *		is set to ENOMEM.
 *	test2:
 *		Invoke mprotect() with an address that is not a multiple
 *		of PAGESIZE.  EINVAL
 *	test3:
 *		Mmap a file with only read permission (PROT_READ).
 *		Try to set write permission (PROT_WRITE) using mprotect(2).
 *		Check that error is set to EACCES.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	03/2002 Paul Larson: case 1 should expect ENOMEM not EFAULT
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"

char *TCID = "mprotect01";
int TST_TOTAL = 3;

struct test_case {
	void *addr;
	int len;
	int prot;
	int error;
	void (*setupfunc) (struct test_case *self);
};

static void cleanup(void);
static void setup(void);
static void setup1(struct test_case *self);
static void setup2(struct test_case *self);
static void setup3(struct test_case *self);

static int fd;

struct test_case TC[] = {
	/* Check for ENOMEM passing memory that cannot be accessed. */
	{NULL, 0, PROT_READ, ENOMEM, setup1},

	/*
	 * Check for EINVAL by passing a pointer which is not a
	 * multiple of PAGESIZE.
	 */
	{NULL, 1024, PROT_READ, EINVAL, setup2},
	/*
	 * Check for EACCES by trying to mark a section of memory
	 * which has been mmap'ed as read-only, as PROT_WRITE
	 */
	{NULL, 0, PROT_WRITE, EACCES, setup3}
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (TC[i].setupfunc != NULL)
				TC[i].setupfunc(&TC[i]);

			TEST(tst_syscall(__NR_mprotect, TC[i].addr, TC[i].len, TC[i].prot));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
	cleanup();
	tst_exit();
}

static void setup1(struct test_case *self)
{
	self->len = getpagesize() + 1;
}

static void setup2(struct test_case *self)
{
	self->addr = malloc(getpagesize());

	if (self->addr == NULL)
		tst_brkm(TINFO, cleanup, "malloc failed");

	/* Ensure addr2 is not page aligned */
	self->addr++;
}

static void setup3(struct test_case *self)
{
	fd = SAFE_OPEN(cleanup, "/dev/zero", O_RDONLY);

	self->len = getpagesize();

	/*
	 * mmap the PAGESIZE bytes as read only.
	 */
	self->addr = mmap(0, self->len, PROT_READ, MAP_SHARED, fd, 0);
	if (self->addr == MAP_FAILED)
		tst_brkm(TBROK, cleanup, "mmap failed");

}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	close(fd);
}
