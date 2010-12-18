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
 * Test Name: accept01
 *
 * Test Description:
 *  Verify that accept() returns the proper errno for various failure cases
 *
 * Usage:  <for command-line>
 *  accept01 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/ioctl.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID = "accept01";	/* Test program identifier.    */
int testno;

int s;				/* socket descriptor */
struct sockaddr_in sin0, fsin1;
socklen_t sinlen;

void setup(void), setup0(void), setup1(void), setup2(void), setup3(void),
cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	struct sockaddr *sockaddr;	/* socket address buffer */
	socklen_t *salen;	/* accept's 3rd argument */
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	void (*setup) (void);
	void (*cleanup) (void);
	char *desc;
} tdat[] = {
	{
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, EBADF, setup0, cleanup0,
		    "bad file descriptor"}, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, ENOTSOCK, setup0, cleanup0,
		    "bad file descriptor"}, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)3,
		    &sinlen, -1, EINVAL, setup1, cleanup1,
		    "invalid socket buffer"}, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&fsin1,
		    (socklen_t *) 1, -1, EINVAL, setup1, cleanup1,
		    "invalid salen"}, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, EINVAL, setup2, cleanup1, "invalid salen"}, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, EINVAL, setup3, cleanup1,
		    "no queued connections"}, {
PF_INET, SOCK_DGRAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, EOPNOTSUPP, setup1, cleanup1, "UDP accept"},};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);	/* Total number of test cases. */

int exp_enos[] = { EBADF, ENOTSOCK, EINVAL, EOPNOTSUPP, 0 };


int main(int ac, char *av[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(accept(s, tdat[testno].sockaddr,
				    tdat[testno].salen));
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_RETURN > 0)
				TEST_RETURN = 0;
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					 " %ld (expected %d), errno %d (expected"
					 " %d)", tdat[testno].desc,
					 TEST_RETURN, tdat[testno].retval,
					 TEST_ERRNO, tdat[testno].experrno);
			} else {
				tst_resm(TPASS, "%s successful",
					 tdat[testno].desc);
			}
			tdat[testno].cleanup();
		}
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	TEST_PAUSE;		/* if -P option specified */

	/* set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* initialize local sockaddr */
	sin0.sin_family = AF_INET;
	sin0.sin_port = 0;
	sin0.sin_addr.s_addr = INADDR_ANY;
}

void cleanup(void)
{
	TEST_CLEANUP;
}

void setup0(void)
{
	if (tdat[testno].experrno == EBADF)
		s = 400;	/* anything not an open file */
	else if ((s = open("/dev/null", O_WRONLY)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "error opening /dev/null");
}

void cleanup0(void)
{
	s = -1;
}

void setup1(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for accept "
			 "test %d: %s", testno, strerror(errno));
	}
	if (bind(s, (struct sockaddr *)&sin0, sizeof(sin0)) < 0) {
		tst_brkm(TBROK, cleanup, "socket bind failed for accept "
			 "test %d: %s", testno, strerror(errno));
	}
	sinlen = sizeof(fsin1);
}

void setup3(void)
{
	int one = 1;

	setup1();
	if (ioctl(s, FIONBIO, &one) < 0) {
		tst_brkm(TBROK, cleanup, "socket ioctl failed for accept "
			 "test %d: %s", testno, strerror(errno));
	}
}

void cleanup1(void)
{
	(void)close(s);
	s = -1;
}

void setup2(void)
{
	setup1();		/* get a socket in s */
	sinlen = 1;		/* invalid s */
}