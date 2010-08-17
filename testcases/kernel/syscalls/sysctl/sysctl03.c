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
 *	sysctl03.c
 *
 * DESCRIPTION
 *	Testcase to check that sysctl(2) sets errno to EPERM correctly.
 *
 * ALGORITHM
 *	a.	Call sysctl(2) as a root user, and attempt to write data
 *		to the kernel_table[]. Since the table does not have write
 *		permissions even for the root, it should fail EPERM.
 *	b.	Call sysctl(2) as a non-root user, and attempt to write data
 *		to the kernel_table[]. Since the table does not have write
 *		permission for the regular user, it should fail with EPERM.
 *
 * NOTE: There is a documentation bug in 2.6.33-rc1 where unfortunately the
 * behavior of sysctl(2) isn't properly documented, as discussed in detail in
 * the following thread:
 * http://sourceforge.net/mailarchive/message.php?msg_name=4B7BA24F.2010705%40linux.vnet.ibm.com.
 *
 * The documentation bug is filed as:
 * https://bugzilla.kernel.org/show_bug.cgi?id=15446 . If you want the message
 * removed, please ask your fellow kernel maintainer to fix his/her
 * documentation.
 *
 * Thanks!
 * -Garrett
 *
 * USAGE:  <for command-line>
 *  sysctl03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	02/2010 Updated by shiwh@cn.fujitsu.com
 *
 * RESTRICTIONS
 *	Test must be run as root.
 */
#include "test.h"
#include "usctest.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>
#include <pwd.h>

char *TCID = "sysctl03";
int TST_TOTAL = 2;
extern int Tst_count;

int sysctl(int *name, int nlen, void *oldval, size_t * oldlenp,
	   void *newval, size_t newlen)
{
	struct __sysctl_args args =
	    { name, nlen, oldval, oldlenp, newval, newlen };
	return syscall(__NR__sysctl, &args);
}

#define SIZE(x) sizeof(x)/sizeof(x[0])
#define OSNAMESZ 100

void setup(void);
void cleanup(void);

int exp_enos[] = { EPERM, 0 };

int main(int ac, char **av)
{
	int exp_eno;
	int lc;
	char *msg;

	char osname[OSNAMESZ];
	int osnamelth, status;
	int name[] = { CTL_KERN, KERN_OSTYPE };
	pid_t pid;
	struct passwd *ltpuser;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	if ((tst_kvercmp(2, 6, 32)) <= 0) {
		exp_eno = EPERM;
	} else {
		/* ^^ Look above this warning. ^^ */
		tst_resm(TWARN, "this test's results are based on potentially undocumented behavior in the kernel. read the NOTE in the source file for more details");
		exp_eno = EACCES;
		exp_enos[0] = EACCES;
	}

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		strcpy(osname, "Linux");
		osnamelth = SIZE(osname);

		TEST(sysctl(name, SIZE(name), 0, 0, osname, osnamelth));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "sysctl(2) succeeded unexpectedly");
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == exp_eno) {
				tst_resm(TPASS|TTERRNO, "Got expected error");
			} else {
				tst_resm(TFAIL|TTERRNO, "Got unexpected error");
			}
		}

		osnamelth = SIZE(osname);
		if ((ltpuser = getpwnam("nobody")) == NULL) {
			tst_brkm(TBROK, cleanup, "getpwnam() failed");
		}

		/* set process ID to "ltpuser1" */
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_brkm(TBROK, cleanup,
				 "seteuid() failed, errno %d", errno);
		}

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* child */
			TEST(sysctl(name, SIZE(name), 0, 0, osname, osnamelth));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);

				if (TEST_ERRNO == exp_eno) {
					tst_resm(TFAIL|TTERRNO,
						"Got expected error");
				} else {
					tst_resm(TPASS|TTERRNO,
						"Got unexpected error");
				}
			}

			cleanup();

		} else {	/* parent */
			/* wait for the child to finish */
			wait(&status);
		}

		/* set process ID back to root */
		if (seteuid(0) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() failed");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* test must be run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
