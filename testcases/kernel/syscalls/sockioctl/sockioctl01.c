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
 * Test Name: sockioctl01
 *
 * Test Description:
 *  Verify that ioctl() on sockets returns the proper errno for various
 *  failure cases
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
#include "safe_macros.h"

char *TCID = "sockioctl01";
int testno;

static int s; /* socket descriptor */
static struct sockaddr_in sin0, fsin1;
static struct ifconf ifc;
static struct ifreq ifr;
static int sinlen;
static int optval;

static char buf[8192];

static void setup(void);
static void setup0(void);
static void setup1(void);
static void setup2(void);
static void setup3(void);

static void cleanup(void);
static void cleanup0(void);
static void cleanup1(void);

struct test_case_t {
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

			TEST(ioctl(s, tdat[testno].cmd, tdat[testno].arg));
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

	sin0.sin_family = AF_INET;
	sin0.sin_port = 0;
	sin0.sin_addr.s_addr = INADDR_ANY;

	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}

static void setup0(void)
{
	if (tdat[testno].experrno == EBADF) {
		s = 1025;	/* anything not an open file */
	} else {
		unlink("test");

		if ((mknod("test", S_IRWXU | S_IFIFO, 0)) == -1) {
			tst_brkm(TBROK, cleanup, "Could not create test - "
				 "errno: %s", strerror(errno));
		}

		if ((s = open("test", O_RDWR)) == -1) {
			tst_brkm(TBROK, cleanup, "Could not open test - "
				 "errno: %s", strerror(errno));
		}

		/*
		 * kernel commit 46ce341b2f176c2611f12ac390adf862e932eb02
		 * changed -EINVAL to -ENOIOCTLCMD, so vfs_ioctl now
		 * returns -ENOTTY.
		 */
		tdat[testno].experrno = ENOTTY;
	}
}

static void cleanup0(void)
{
	if (tdat[testno].experrno != EBADF) {
		(void)close(s);
		s = -1;
	}
}

static void setup1(void)
{
	s = SAFE_SOCKET(cleanup, tdat[testno].domain, tdat[testno].type,
			tdat[testno].proto);
	SAFE_BIND(cleanup, s, (struct sockaddr *)&sin0, sizeof(sin0));
	sinlen = sizeof(fsin1);

	if (strncmp(tdat[testno].desc, "ATMARK on UDP", 14) == 0)
		tdat[testno].experrno = ENOTTY;
}

static void setup2(void)
{
	s = SAFE_SOCKET(cleanup, tdat[testno].domain, tdat[testno].type,
			tdat[testno].proto);
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
}

static void setup3(void)
{
	setup2();
	SAFE_IOCTL(cleanup, s, SIOCGIFCONF, &ifc);
	ifr = *(struct ifreq *)ifc.ifc_buf;
}

static void cleanup1(void)
{
	(void)close(s);
	s = -1;
}
