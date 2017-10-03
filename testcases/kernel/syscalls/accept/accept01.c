/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
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
 * Description:
 *  Verify that accept() returns the proper errno for various failure cases
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>

#include <netinet/in.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "accept01";
int testno;

int s;				/* socket descriptor */
struct sockaddr_in sin0, fsin1;
socklen_t sinlen;

static void setup(void);
static void cleanup(void);
static void setup0(void);
static void cleanup0(void);
static void setup1(void);
static void cleanup1(void);
static void setup2(void);
static void setup3(void);

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
		    &sinlen, -1, EINVAL, setup2, cleanup1,
		    "invalid salen"}, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, EINVAL, setup3, cleanup1,
		    "no queued connections"}, {
	PF_INET, SOCK_DGRAM, 0, (struct sockaddr *)&fsin1,
		    &sinlen, -1, EOPNOTSUPP, setup1, cleanup1,
		    "UDP accept"},};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int ac, char *av[])
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(accept(s, tdat[testno].sockaddr,
				    tdat[testno].salen));
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

static void setup(void)
{
	TEST_PAUSE;

	/* initialize local sockaddr */
	sin0.sin_family = AF_INET;
	sin0.sin_port = 0;
	sin0.sin_addr.s_addr = INADDR_ANY;
}

static void cleanup(void)
{
}

static void setup0(void)
{
	if (tdat[testno].experrno == EBADF)
		s = 400;	/* anything not an open file */
	else if ((s = open("/dev/null", O_WRONLY)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "error opening /dev/null");
}

static void cleanup0(void)
{
	s = -1;
}

static void setup1(void)
{
	s = SAFE_SOCKET(cleanup, tdat[testno].domain, tdat[testno].type,
			tdat[testno].proto);
	SAFE_BIND(cleanup, s, (struct sockaddr *)&sin0, sizeof(sin0));
	sinlen = sizeof(fsin1);
}

static void cleanup1(void)
{
	(void)close(s);
	s = -1;
}

static void setup2(void)
{
	setup1();		/* get a socket in s */
	sinlen = 1;		/* invalid s */
}

static void setup3(void)
{
	int one = 1;

	setup1();
	SAFE_IOCTL(cleanup, s, FIONBIO, &one);
}
