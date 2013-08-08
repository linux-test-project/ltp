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
 * DESCRIPTION
 *	Verify that mount(2) returns -1 and sets errno to  EPERM if the user
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
#include <pwd.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);

char *TCID = "mount04";

#define DIR_MODE	S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP

static char *mntpoint = "mntpoint";
static char *fstype = "ext2";
static char *device;
static int Dflag = 0;

static struct test_case_t {
	char *err_desc;		/* error description            */
	int exp_errno;		/* Expected error no            */
	char *exp_errval;	/* Expected error value string  */
} testcases[] = {
	{
	"User not Super User/root", EPERM, "EPERM"}
};

int TST_TOTAL = sizeof(testcases) / sizeof(testcases[0]);

static int exp_enos[] = { EPERM, 0 };

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &Dflag, &device},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc, i;
	char *msg;

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (Dflag == 0) {
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option.");
		tst_exit();
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

		for (i = 0; i < TST_TOTAL; ++i) {

			TEST(mount(device, mntpoint, fstype, 0, NULL));

			/* check return code */
			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases[i].exp_errno)) {
				tst_resm(TPASS, "mount(2) expected failure; "
					 "Got errno - %s : %s",
					 testcases[i].exp_errval,
					 testcases[i].err_desc);
			} else {
				if (umount(mntpoint) == -1) {
					tst_brkm(TBROK, cleanup, "umount(2) "
						 "failed to umount mntpoint %s "
						 "errno - %d : %s", mntpoint,
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
				tst_resm(TFAIL, "mount(2) failed to produce "
					 "expected error; %d, errno:%s got %d",
					 testcases[i].exp_errno,
					 testcases[i].exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);
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

	ltpuser = getpwnam(nobody_uid);
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, cleanup,
			 "seteuid() failed to change euid to %d "
			 "errno = %d : %s", ltpuser->pw_uid, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}

	tst_tmpdir();

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) failed; "
			 "errno = %d: %s", mntpoint, DIR_MODE, errno,
			 strerror(errno));
	}

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	return;
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();

	/* Set effective user id back to root */
	if (seteuid(0) == -1) {
		tst_resm(TINFO, "seteuid failed to "
			 "to set the effective uid to root");
		perror("seteuid");
	}

	return;
}

static void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
