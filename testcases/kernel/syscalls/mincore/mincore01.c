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
#include "usctest.h"

static int PAGESIZE;
static rlim_t STACK_LIMIT = 10485760;

static void cleanup(void);
static void setup(void);
static void setup1(void);
static void setup2(void);
static void setup3(void);

char *TCID = "mincore01";
int TST_TOTAL = 3;

static char *global_pointer = NULL;
static unsigned char *global_vec = NULL;
static int global_len = 0;
static int fd = 0;

static struct test_case_t {
	char *addr;
	int len;
	unsigned char *vector;
	int exp_errno;
	void (*setupfunc) (void);
} TC[] = {
	{NULL, 0, NULL, EINVAL, setup1},
	{NULL, 0, NULL, EFAULT, setup2},
	{NULL, 0, NULL, ENOMEM, setup3},
};

int main(int ac, char **av)
{
	int lc;
	int i;
	char *msg;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, cleanup, "error parsing options: %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (TC[i].setupfunc != NULL)
				TC[i].setupfunc();
			
			TEST(mincore(TC[i].addr, TC[i].len, TC[i].vector));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].exp_errno) {
				tst_resm(TPASS, "expected failure: "
					 "errno = %d (%s)", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error %d (%s), "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].exp_errno);
			}
		}
	}
	cleanup();
	tst_exit();
}

static void setup1(void)
{
	TC[0].addr = global_pointer + 1;
	TC[0].len = global_len;
	TC[0].vector = global_vec;
}

void setup2(void)
{
	unsigned char *t;
	struct rlimit limit;
	    
	t = mmap(NULL, global_len, PROT_READ | PROT_WRITE,
	         MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	/* Create pointer to invalid address */
	if (t == MAP_FAILED) {
		tst_brkm(TBROK | TERRNO, cleanup,
		         "mmaping anonymous memory failed");
	}

	munmap(t, global_len);

	/* set stack limit so that the unmaped pointer is invalid for architectures like s390 */
	limit.rlim_cur = STACK_LIMIT;
	limit.rlim_max = STACK_LIMIT;

	if (setrlimit(RLIMIT_STACK, &limit) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "setrlimit failed");

	TC[1].addr = global_pointer;
	TC[1].len = global_len;
	TC[1].vector = t;
}

static void setup3(void)
{
	TC[2].addr = global_pointer;
	TC[2].len = global_len * 2;
	munmap(global_pointer + global_len, global_len);
	TC[2].vector = global_vec;
}

static void setup(void)
{
	char *buf;

	PAGESIZE = getpagesize();

	tst_tmpdir();

	/* global_pointer will point to a mmapped area of global_len bytes */
	global_len = PAGESIZE * 2;

	buf = malloc(global_len);
	memset(buf, 42, global_len);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* create a temporary file */
	fd = open("mincore01", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Error while creating temporary file");
	}

	/* fill the temporary file with two pages of data */
	if (write(fd, buf, global_len) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Error while writing to temporary file");
	}
	free(buf);

	/* map the file in memory */
	global_pointer = mmap(NULL, global_len * 2,
	                      PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED,
	                      fd, 0);

	if (global_pointer == MAP_FAILED) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Temporary file could not be mmapped");
	}

	/* initialize the vector buffer to collect the page info */
	global_vec = malloc((global_len + PAGESIZE - 1) / PAGESIZE);
}

static void cleanup(void)
{
	TEST_CLEANUP;

	free(global_vec);
	munmap(global_pointer, global_len);
	close(fd);
	tst_rmdir();
}
