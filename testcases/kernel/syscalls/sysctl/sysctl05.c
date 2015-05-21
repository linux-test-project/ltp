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
 * NAME
 *	sysctl05.c
 *
 * DESCRIPTION
 *	Testcase to check that sysctl(2) sets errno to EFAULT
 *
 * ALGORITHM
 *	1. Call sysctl(2) with the address of sc_oldname outside the address
 *	   space of the process, and expect EFAULT.
 *	2. Call sysctl(2) with the address of sc_oldval outside the address
 *	   space of the process, and expect EFAULT.
 *
 * USAGE:  <for command-line>
 *  sysctl05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include "test.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>
#include <linux/version.h>
#include <errno.h>

char *TCID = "sysctl05";

/* This is an older/deprecated syscall that newer arches are omitting */
#ifdef __NR_sysctl

int TST_TOTAL = 2;

int sysctl(int *name, int nlen, void *oldval, size_t * oldlenp,
	   void *newval, size_t newlen)
{
	struct __sysctl_args args =
	    { name, nlen, oldval, oldlenp, newval, newlen };
	return syscall(__NR__sysctl, &args);
}

char osname[BUFSIZ];
size_t osnamelth;

void setup(void);
void cleanup(void);

struct testcases {
	char *desc;
	int name[2];
	int size;
	void *oldval;
	size_t *oldlen;
	void *newval;
	int newlen;
	int (*cleanup) ();
	int exp_retval;
	int exp_errno;
} testcases[] = {
	{
		"Test for EFAULT: invalid oldlen", {
	CTL_KERN, KERN_OSRELEASE},
		    2, osname, (void *)-1, NULL, 0, NULL, -1, EFAULT}, {
		"Test for EFAULT: invalid oldval", {
	CTL_KERN, KERN_VERSION},
		    2, (void *)-1, &osnamelth, NULL, 0, NULL, -1, EFAULT}
};

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;
	int i;
	int ret = 0;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			osnamelth = sizeof(osname);

			TEST(sysctl(testcases[i].name, testcases[i].size,
				    testcases[i].oldval, testcases[i].oldlen,
				    testcases[i].newval, testcases[i].newlen));

			if (TEST_RETURN != testcases[i].exp_retval) {
				tst_resm(TFAIL, "sysctl(2) returned unexpected "
					 "retval, expected: %d, got: %d",
					 testcases[i].exp_retval, ret);
				continue;
			}

			if (TEST_ERRNO == ENOSYS) {
				tst_resm(TCONF,
					 "You may need to make CONFIG_SYSCTL_SYSCALL=y"
					 " to your kernel config.");
			} else if (TEST_ERRNO != testcases[i].exp_errno) {
				tst_resm(TFAIL, "sysctl(2) returned unexpected "
					 "errno, expected: %d, got: %d",
					 testcases[i].exp_errno, errno);
			} else {
				tst_resm(TPASS, "sysctl(2) set errno correctly "
					 "to %d", testcases[i].exp_errno);
			}

			if (testcases[i].cleanup) {
				(void)testcases[i].cleanup();
			}
		}
	}
	cleanup();

	tst_exit();
}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

}

#else
int TST_TOTAL = 0;

int main(void)
{

	tst_brkm(TCONF, NULL,
		 "This test needs a kernel that has sysctl syscall.");
}
#endif
