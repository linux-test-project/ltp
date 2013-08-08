/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    DESCRIPTION
 *	Verify that umount(2) returns -1 and sets errno to  EPERM if the user
 *	is not the super-user.
 *
 * RESTRICTIONS
 *	test must be run with the -D option
 *	test doesn't support -c option to run it in parallel, as mount
 *	syscall is not supposed to run in parallel.
 */

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);
static void cleanup1(void);

char *TCID = "umount03";

#define FSTYPE_LEN	20
#define DIR_MODE	S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

static char *mntpoint = "mntpoint";
static char *fstype = "ext2";
static char *device;
static int Dflag = 0;

static struct test_case_t {
	char *err_desc;		/* error description            */
	int exp_errno;		/* Expected error no            */
	char *exp_errval;	/* Expected error value string  */
} testcases = {
"User not Super User/root", EPERM, "EPERM"};

int TST_TOTAL = 1;

static int exp_enos[] = { EPERM, 0 };

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &Dflag, &device},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	char *msg;
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;
	int status;

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (Dflag == 0) {
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option, Run '%s  -h' for option "
			 " information.", TCID);
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		switch (fork()) {

		case -1:
			tst_resm(TWARN, "fork() failed");
			continue;

		case 0:
			if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
				tst_brkm(TBROK, NULL, "\"nobody\" user"
					 "not present");
			}
			if (setuid(ltpuser->pw_uid) == -1) {
				tst_resm(TWARN, "setuid failed "
					 "to set the effective uid to %d",
					 ltpuser->pw_uid);
				exit(1);
			}
			TEST(umount(mntpoint));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases.exp_errno)) {
				tst_resm(TPASS, "umount(2) expected failure "
					 "Got errno - %s : %s",
					 testcases.exp_errval,
					 testcases.err_desc);
			} else {
				tst_resm(TFAIL, "umount(2) failed to produce "
					 "expected error; %d, errno:%s got %d",
					 testcases.exp_errno,
					 testcases.exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);
			exit(1);

		default:
			if ((wait(&status)) < 0) {
				tst_resm(TFAIL, "wait() failed");
			}
		}

	}

	cleanup();
	tst_exit();

}

static void setup(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);
	tst_mkfs(NULL, device, fstype, NULL);

	if ((ltpuser = getpwnam(nobody_uid)) == NULL)
		tst_brkm(TBROK, NULL, "\"nobody\" user not present");

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, NULL, "setuid failed to set the "
			 "effective uid to %d", ltpuser->pw_uid);
	}

	tst_tmpdir();

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK, cleanup1, "mkdir(%s, %#o) failed; errno = %d:"
			 " %s", mntpoint, DIR_MODE, errno, strerror(errno));
	}

	if (seteuid(0) == -1) {
		tst_brkm(TBROK, cleanup1, "setuid failed to set the effective"
			 " uid to %d", ltpuser->pw_uid);
	}
	TEST_EXP_ENOS(exp_enos);

	if (access(device, F_OK))
		tst_brkm(TBROK, cleanup1, "Device '%s' does not exist", device);

	TEST(mount(device, mntpoint, fstype, 0, NULL));

	if (TEST_RETURN != 0) {
		tst_brkm(TBROK, cleanup1, "mount(2) failed to mount device %s "
			 "errno = %d : %s", device, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}

	TEST_PAUSE;
}

static void cleanup1(void)
{
	TEST_CLEANUP;
	tst_rmdir();
	tst_exit();
}

static void cleanup(void)
{
	TEST(umount(mntpoint));
	if (TEST_RETURN != 0) {
		tst_resm(TWARN, "umount(2) failed to umount device %s while"
			 " cleanuo errno = %d : %s", mntpoint, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}

	TEST_CLEANUP;
	tst_rmdir();
}

static void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
