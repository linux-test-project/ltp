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
 *	mprotect01.c
 *
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
 * USAGE:  <for command-line>
 *  mprotect01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	03/2002 Paul Larson: case 1 should expect ENOMEM not EFAULT
 *
 * RESTRICTIONS
 *	None
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <limits.h>		/* for PAGESIZE */
#include <unistd.h>
#include "test.h"
#include "usctest.h"

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

void cleanup(void);
void setup(void);
void setup1(void);
void setup2(void);
void setup3(void);

char *TCID = "mprotect01";
int TST_TOTAL = 3;

void *addr1, *addr2, *addr3;
int fd;

int exp_enos[] = { ENOMEM, EINVAL, EACCES, 0 };

struct test_case_t {
	void **addr;
	int len;
	int prot;
	int error;
	void (*setupfunc) ();
} TC[] = {
#ifdef __ia64__
	/* Check for ENOMEM passing memory that cannot be accessed. */
	{
	&addr1, 1024, PROT_READ, ENOMEM, setup1},
#else
	/* Check for ENOMEM passing memory that cannot be accessed. */
	{
	&addr1, 1024, PROT_READ, ENOMEM, NULL},
#endif
	    /*
	     * Check for EINVAL by passing a pointer which is not a
	     * multiple of PAGESIZE.
	     */
	{
	&addr2, 1024, PROT_READ, EINVAL, setup2},
	    /*
	     * Check for EACCES by trying to mark a section of memory
	     * which has been mmap'ed as read-only, as PROT_WRITE
	     */
	{
	&addr3, PAGESIZE, PROT_WRITE, EACCES, setup3}
};

#ifndef UCLINUX

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int i;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

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

			TEST(mprotect(*(TC[i].addr), TC[i].len, TC[i].prot));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

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
		close(fd);
	}
	cleanup();
	tst_exit();
 }

#else

int main()
{
	tst_resm(TINFO, "Ignore this test on uClinux");
	tst_exit();
}

#endif /* UCLINUX */

/*
 * setup1() - sets up conditions for the first test
 */
void setup1()
{
	TC[0].len = getpagesize() + 1;
}

/*
 * setup2() - sets up conditions for the second test
 */
void setup2()
{

	addr2 = (char *)malloc(PAGESIZE);
	if (addr2 == NULL) {
		tst_brkm(TINFO, cleanup, "malloc failed");
	 }
	addr2++;		/* Ensure addr2 is not page aligned */
}

/*
 * setup3() - sets up conditions for the third test
 */
void setup3()
{
	fd = open("/etc/passwd", O_RDONLY);
	if (fd < 0) {
		tst_brkm(TBROK, cleanup, "open failed");
	 }

	/*
	 * mmap the PAGESIZE bytes as read only.
	 */
	addr3 = mmap(0, PAGESIZE, PROT_READ, MAP_SHARED, fd, 0);
	if (addr3 < 0) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	 }
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
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

}