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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: bind01
 *
 * Test Description:
 *  Verify that bind() returns the proper errno for various failure cases
 *
 * Usage:  <for command-line>
 *  bind01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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
#include <sys/un.h>

#include <netinet/in.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "bind01";
int testno;

int s;				/* socket descriptor */
struct sockaddr_in sin1, sin2, sin3;
struct sockaddr_un sun1;

void setup(void), setup0(void), setup1(void), setup2(void),
cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	struct sockaddr *sockaddr;	/* socket address buffer */
	int salen;		/* bind's 3rd argument */
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	void (*setup) (void);
	void (*cleanup) (void);
	char *desc;
} tdat[] = {
#ifndef UCLINUX
/* Skip since uClinux does not implement memory protection */
	{
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)-1,
		    sizeof(struct sockaddr_in), -1, EFAULT, setup0,
		    cleanup0, "invalid sockaddr"},
#endif
	{
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin1,
		    3, -1, EINVAL, setup0, cleanup0, "invalid salen"}, {
	0, 0, 0, (struct sockaddr *)&sin1,
		    sizeof(sin1), -1, ENOTSOCK, setup1, cleanup1,
		    "invalid socket"}
	, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin2,
		    sizeof(sin2), 0, 0, setup0, cleanup0, "INADDR_ANYPORT"}
	, {
	PF_UNIX, SOCK_STREAM, 0, (struct sockaddr *)&sun1,
		    sizeof(sun1), -1, EADDRINUSE, setup0, cleanup0,
		    "UNIX-domain of current directory"}
	, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin3,
		    sizeof(sin3), -1, EADDRNOTAVAIL, setup0, cleanup0,
		    "non-local address"}
,};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(bind
			     (s, tdat[testno].sockaddr, tdat[testno].salen));
			if (TEST_RETURN > 0) {
				TEST_RETURN = 0;
			} else {
			}
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

	TEST_PAUSE;		/* if -p option specified */

	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
	/* this port must be unused! */
	sin1.sin_port = tst_get_unused_port(NULL, AF_INET, SOCK_STREAM);
	sin1.sin_addr.s_addr = INADDR_ANY;

	sin2.sin_family = AF_INET;
	sin2.sin_port = 0;
	sin2.sin_addr.s_addr = INADDR_ANY;

	sin3.sin_family = AF_INET;
	sin3.sin_port = 0;
	/* assumes 10.255.254.253 is not a local interface address! */
	sin3.sin_addr.s_addr = htonl(0x0AFFFEFD);

	sun1.sun_family = AF_UNIX;
	strncpy(sun1.sun_path, ".", sizeof(sun1.sun_path));

}

void cleanup(void)
{
}

void setup0(void)
{
	s = SAFE_SOCKET(cleanup, tdat[testno].domain, tdat[testno].type,
		        tdat[testno].proto);
}

void cleanup0(void)
{
	(void)close(s);
}

void setup1(void)
{
	/* setup for the "not a socket" case */
	if ((s = open("/dev/null", O_WRONLY)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "open(/dev/null) failed");

}

void cleanup1(void)
{
	s = -1;
}
