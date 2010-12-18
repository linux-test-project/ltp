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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	mincore01.c
 *
 * DESCRIPTION
 *	Testcase to check the error conditions for mincore
 *
 * ALGORITHM
 *	test1:
 *		Invoke mincore() when the start address is not multiple of page size.
 *		EINVAL
 *	test2:
 *		Invoke mincore() when the vector points to an invalid address. EFAULT
 *	test3:
 *		Invoke mincore() when the starting address + length contained unmapped
 *		memory. ENOMEM
 *
 * USAGE:  <for command-line>
 *  mincore01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *  Author: Rajeev Tiwari: rajeevti@in.ibm.com
 *	08/2004 Rajeev Tiwari : does a basic sanity check for the various error
 *  conditions possible with the mincore system call.
 *
 * 	2004/09/10 Gernot Payer <gpayer@suse.de>
 * 		code cleanup
 *$
 * RESTRICTIONS
 *	None
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

static char file_name[] = "fooXXXXXX";
static char *global_pointer = NULL;
static unsigned char *global_vec = NULL;
static int global_len = 0;
static int file_desc = 0;

#if !defined(UCLINUX)

static struct test_case_t {
	char *addr;
	int len;
	unsigned char *vector;
	int exp_errno;
	void (*setupfunc) ();
} TC[] = {
	{
	NULL, 0, NULL, EINVAL, setup1}, {
	NULL, 0, NULL, EFAULT, setup2}, {
NULL, 0, NULL, ENOMEM, setup3},};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int i;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, cleanup, "error parsing options: %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			/* perform test specific setup */
			if (TC[i].setupfunc != NULL) {
				TC[i].setupfunc();
			}
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

/*
 * setup1() - sets up conditions for the first test. the start address is not
 * multiple of page size
 */
void setup1()
{
	TC[0].addr = global_pointer + 1;
	TC[0].len = global_len;
	TC[0].vector = global_vec;
}

/*
 * setup2() - sets up conditions for the test 2. the vector points to an
 * invalid address.
 */
void setup2()
{
	unsigned char *t;
	struct rlimit limit;

	/* Create pointer to invalid address */
	if (MAP_FAILED ==
	    (t =
	     mmap(0, global_len, PROT_READ | PROT_WRITE,
		  MAP_PRIVATE | MAP_ANONYMOUS, 0, 0))) {
		tst_brkm(TBROK, cleanup, "mmaping anonymous memory failed: %s",
			 strerror(errno));
	}
	munmap(t, global_len);

	/* set stack limit so that the unmaped pointer is invalid for architectures like s390 */
	limit.rlim_cur = STACK_LIMIT;
	limit.rlim_max = STACK_LIMIT;

	if (setrlimit(RLIMIT_STACK, &limit) != 0) {
		tst_brkm(TBROK, cleanup, "setrlimit failed: %s",
			 strerror(errno));
	}

	TC[1].addr = global_pointer;
	TC[1].len = global_len;
	TC[1].vector = t;
}

/*
 *  setup3() - performs the setup for test3(the starting address + length
 *  contained unmapped memory). we give the length of mapped file equal to 5
 *  times the mapped file size.
 */
void setup3()
{
	TC[2].addr = global_pointer;
	TC[2].len = global_len * 2;
	munmap(global_pointer + global_len, global_len);
	TC[2].vector = global_vec;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	char *buf;

	PAGESIZE = getpagesize();

	/* global_pointer will point to a mmapped area of global_len bytes */
	global_len = PAGESIZE * 2;

	buf = (char *)malloc(global_len);
	memset(buf, 42, global_len);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* create a temporary file */
	if (-1 == (file_desc = mkstemp(file_name))) {
		tst_brkm(TBROK, cleanup,
			 "Error while creating temporary file: %s",
			 strerror(errno));
	}

	/* fill the temporary file with two pages of data */
	if (-1 == write(file_desc, buf, global_len)) {
		tst_brkm(TBROK, cleanup,
			 "Error while writing to temporary file: %s",
			 strerror(errno));
	}
	free(buf);

	/* map the file in memory */
	if (MAP_FAILED ==
	    (global_pointer =
	     (char *)mmap(NULL, global_len * 2,
			  PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED,
			  file_desc, 0))) {
		tst_brkm(TBROK, cleanup,
			 "Temporary file could not be mmapped: %s",
			 strerror(errno));
	}

	/* initialize the vector buffer to collect the page info */
	global_vec = malloc((global_len + PAGESIZE - 1) / PAGESIZE);
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup()
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	free(global_vec);
	munmap(global_pointer, global_len);
	close(file_desc);
	remove(file_name);

}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */