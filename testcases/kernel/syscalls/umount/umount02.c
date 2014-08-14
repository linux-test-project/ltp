/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 *	Check for basic errors returned by umount(2) system call.
 *
 *	Verify that umount(2) returns -1 and sets errno to
 *
 *	1) EBUSY if it cannot be umounted, because dir is still busy.
 *	2) EFAULT if specialfile or device file points to invalid address space.
 *	3) ENOENT if pathname was empty or has a nonexistent component.
 *	4) EINVAL if specialfile or device is invalid or not a mount point.
 *	5) ENAMETOOLONG if pathname was longer than MAXPATHLEN.
 *
 * RESTRICTIONS
 *	test must be run with the -D option
 *	test doesn't support -c option to run it in parallel, as mount
 *	syscall is not supposed to run in parallel.
 *****************************************************************************/

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

static void help(void);
static void setup(void);
static void cleanup(void);

char *TCID = "umount02";

#define DIR_MODE	S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
#define FILE_MODE	S_IRWXU | S_IRWXG | S_IRWXO
#define MNTPOINT        "mntpoint"

static char long_path[PATH_MAX + 2];
static int mount_flag;
static int fd;

static char *fstype;
static char *device;
static int Dflag = 0;

static struct test_case_t {
	char *err_desc;
	char *mntpoint;
	int exp_errno;
	char *exp_retval;
} testcases[] = {
	{"Already mounted/busy", MNTPOINT, EBUSY, "EBUSY"},
	{"Invalid address space", NULL, EFAULT, "EFAULT"},
	{"Directory not found", "nonexistent", ENOENT, "ENOENT"},
	{"Invalid  device", "./", EINVAL, "EINVAL"},
	{"Pathname too long", long_path, ENAMETOOLONG, "ENAMETOOLONG"}
};

int TST_TOTAL = ARRAY_SIZE(testcases);

static int exp_enos[] = { EBUSY, EINVAL, EFAULT, ENAMETOOLONG, ENOENT, 0 };

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &Dflag, &device},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc, i;
	const char *msg;

	if ((msg = parse_opts(ac, av, options, &help)))
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (Dflag == 0) {
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option, Run '%s  -h' for option "
			 " information.", TCID);
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			TEST(umount(testcases[i].mntpoint));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases[i].exp_errno)) {
				tst_resm(TPASS, "umount(2) expected failure; "
					 "Got errno - %s : %s",
					 testcases[i].exp_retval,
					 testcases[i].err_desc);
			} else {
				tst_resm(TFAIL, "umount(2) failed to produce "
					 "expected error; %d, errno:%s got %d",
					 testcases[i].exp_errno,
					 testcases[i].exp_retval, TEST_ERRNO);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);
	tst_mkfs(NULL, device, fstype, NULL);

	tst_tmpdir();

	memset(long_path, 'a', PATH_MAX + 1);

	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);

	if (mount(device, MNTPOINT, fstype, 0, NULL))
		tst_brkm(TBROK | TERRNO, cleanup, "mount() failed");
	mount_flag = 1;

	fd = SAFE_OPEN(cleanup, MNTPOINT "/file", O_CREAT | O_RDWR);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	if (mount_flag && umount(MNTPOINT))
		tst_resm(TWARN | TERRNO, "umount() failed");

	tst_rmdir();
}

static void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
