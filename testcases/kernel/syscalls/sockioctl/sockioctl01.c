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
 * Test Name: sockioctl01
 *
 * Test Description:
 *  Verify that ioctl() on sockets returns the proper errno for various
 *  failure cases
 *
 * Usage:  <for command-line>
 *  sockioctl01 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
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
#include <sys/stat.h>

#include <netinet/in.h>
#include <net/if.h>

#include "test.h"
#include "usctest.h"

char *TCID = "sockioctl01";	/* Test program identifier.    */
int testno;

int s;				/* socket descriptor */
struct sockaddr_in sin0, fsin1;
struct ifconf ifc;
struct ifreq ifr;
int sinlen;
int optval;
int exp_enos[] = { EBADF, EINVAL, EFAULT, 0 };

char buf[8192];

void setup(void), setup0(void), setup1(void), setup2(void), setup3(void),
cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	int cmd;		/* IPPROTO_* */
	void *arg;
	struct sockaddr *sin;
	int salen;
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	void (*setup) (void);
	void (*cleanup) (void);
	char *desc;
} tdat[] = {
	{
	PF_INET, SOCK_STREAM, 0, SIOCATMARK, &optval,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), -1,
		    EBADF, setup0, cleanup0, "bad file descriptor"}
	, {
	PF_INET, SOCK_STREAM, 0, SIOCATMARK, &optval,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), -1,
		    EINVAL, setup0, cleanup0, "not a socket"}
	,
#if !defined(UCLINUX)
	{
	PF_INET, SOCK_STREAM, 0, SIOCATMARK, 0,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), -1,
		    EFAULT, setup1, cleanup1, "invalid option buffer"}
	,
#endif
	{
	PF_INET, SOCK_DGRAM, 0, SIOCATMARK, &optval,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), -1,
		    EINVAL, setup1, cleanup1, "ATMARK on UDP"}
	, {
	PF_INET, SOCK_DGRAM, 0, SIOCGIFCONF, &ifc,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), 0,
		    0, setup2, cleanup1, "SIOCGIFCONF"}
	, {
	PF_INET, SOCK_DGRAM, 0, SIOCGIFFLAGS, &ifr,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), 0,
		    0, setup3, cleanup1, "SIOCGIFFLAGS"}
	, {
	PF_INET, SOCK_DGRAM, 0, SIOCGIFFLAGS, 0,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), -1,
		    EFAULT, setup3, cleanup1, "SIOCGIFFLAGS with invalid ifr"}
	, {
	PF_INET, SOCK_DGRAM, 0, SIOCSIFFLAGS, 0,
		    (struct sockaddr *)&fsin1, sizeof(fsin1), -1,
		    EFAULT, setup3, cleanup1, "SIOCSIFFLAGS with invalid ifr"}
,};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);	/* Total number of test cases. */


int main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(ioctl(s, tdat[testno].cmd, tdat[testno].arg));
			TEST_ERROR_LOG(TEST_ERRNO);
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
	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;		/* if -P option specified */

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
		s = 1025;	/* anything not an open file */
	else {
		tst_tmpdir();
		if ((mknod("test", O_RDWR | O_CREAT | S_IFIFO, 0)) == -1)
			tst_brkm(TBROK, cleanup, "Could not create test - "
				 "errno: %s", strerror(errno));
		if ((s = open("test", O_RDWR)) == -1)
			tst_brkm(TBROK, cleanup, "Could not open test - "
				 "errno: %s", strerror(errno));
	}
}

void cleanup0(void)
{
	/* delete the test directory created in setup0() */
	if (tdat[testno].experrno != EBADF) {
		(void)close(s);
		s = -1;
		tst_rmdir();
	}
}

void setup1(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			 strerror(errno));
	}
	if (bind(s, (struct sockaddr *)&sin0, sizeof(sin0)) < 0) {
		tst_brkm(TBROK, cleanup, "socket bind failed for: %s",
			 strerror(errno));
	}
	sinlen = sizeof(fsin1);
}

void setup2(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			 strerror(errno));
	}
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
}

void setup3(void)
{
	setup2();
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			 strerror(errno));
	}
	ifr = *(struct ifreq *)ifc.ifc_buf;
}

void cleanup1(void)
{
	(void)close(s);
	s = -1;
}