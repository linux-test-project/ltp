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
 *	sysctl01.c
 *
 * DESCRIPTION
 *	Testcase for testing the basic functionality of sysctl(2) system call.
 *	This testcase attempts to read the kernel parameters using
 *	sysctl({CTL_KERN, KERN_* }, ...) and compares it with the known
 *	values.
 *
 * USAGE:  <for command-line>
 *  sysctl01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
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
#include <errno.h>
#include <unistd.h>
#include <linux/version.h>
#include <sys/utsname.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>

char *TCID = "sysctl01";

/* This is an older/deprecated syscall that newer arches are omitting */
#ifdef __NR_sysctl

int TST_TOTAL = 3;

static int sysctl(int *name, int nlen, void *oldval, size_t * oldlenp,
		  void *newval, size_t newlen)
{
	struct __sysctl_args args =
	    { name, nlen, oldval, oldlenp, newval, newlen };
	return syscall(__NR__sysctl, &args);
}

struct utsname buf;
char osname[BUFSIZ];
size_t osnamelth;

void setup(void);
void cleanup(void);

struct test_case_t {
	char *desc;
	int name[2];
	int size;
	char *oldval;
	size_t *oldlen;
	void *newval;
	int newlen;
	int (*cleanup) ();
	int exp_retval;
} TC[] = {
	{
		"Test for KERN_OSTYPE", {
	CTL_KERN, KERN_OSTYPE}, 2, osname, &osnamelth, NULL, 0, NULL, 0}, {
		"Test for KERN_OSRELEASE", {
	CTL_KERN, KERN_OSRELEASE}, 2,
		    osname, &osnamelth, NULL, 0, NULL, 0}, {
		"Test for KERN_VERSION", {
	CTL_KERN, KERN_VERSION}, 2, osname, &osnamelth, NULL, 0, NULL, 0}
};

int main(int ac, char **av)
{
	int lc;
	int i;
	char *comp_string;

	comp_string = NULL;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			osnamelth = sizeof(osname);

			switch (i) {
			case 0:
				comp_string = buf.sysname;
				break;
			case 1:
				comp_string = buf.release;
				break;
			case 2:
				comp_string = buf.version;
				break;
			}

			TEST(sysctl(TC[i].name, TC[i].size, TC[i].oldval,
				    TC[i].oldlen, TC[i].newval, TC[i].newlen));

			if (TEST_RETURN != 0) {
				if (TEST_ERRNO == ENOSYS) {
					tst_resm(TCONF,
						 "You may need to make CONFIG_SYSCTL_SYSCALL=y"
						 " to your kernel config.");
				} else {
					tst_resm(TFAIL,
						 "sysctl(2) failed unexpectedly "
						 "errno:%d", TEST_ERRNO);
				}
				continue;
			}

			if (strcmp(TC[i].oldval, comp_string) != 0) {
				tst_resm(TFAIL, "strings don't match - %s : %s",
					 TC[i].oldval, comp_string);
			} else {
				tst_resm(TPASS, "%s is correct", TC[i].desc);
			}
			if (TC[i].cleanup) {
				(void)TC[i].cleanup();
			}
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* get kernel name and information */
	if (uname(&buf) == -1) {
		tst_brkm(TBROK, cleanup, "uname() failed");
	}
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
