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

static void help(void);
static void setup(void);
static void cleanup(void);

static int setup_test(int, int);
static void cleanup_test(int);

char *TCID = "umount02";

#define FSTYPE_LEN	20
#define DIR_MODE	S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
#define FILE_MODE	S_IRWXU | S_IRWXG | S_IRWXO

static char *Einval = "nonexixstent";
static char Longpathname[PATH_MAX + 2];
static char Path[PATH_MAX];
static char *Fstype;
static char *Device;
static char *Mntpoint;
static unsigned long Flag;

static int fd;
static char *mntpoint = "mntpoint";
static char *fstype;
static char *device;
static int Dflag = 0;

static struct test_case_t {
	char *err_desc;		/* error description            */
	int exp_errno;		/* Expected error no            */
	char *exp_errval;	/* Expected error value string  */
} testcases[] = {
	{
	"Already mounted/busy", EBUSY, "EBUSY"}, {
	"Invalid address space", EFAULT, "EFAULT"}, {
	"Directory not found", ENOENT, "ENOENT"}, {
	"Invalid  device ", EINVAL, "EINVAL"}, {
	"Pathname too long", ENAMETOOLONG, "ENAMETOOLONG"}
};

int TST_TOTAL = sizeof(testcases) / sizeof(testcases[0]);

static int exp_enos[] = { EBUSY, EINVAL, EFAULT, ENAMETOOLONG, ENOENT, 0 };

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &Dflag, &device},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc, i;
	char *msg;

	if ((msg = parse_opts(ac, av, options, &help)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Check for mandatory option of the testcase */
	if (Dflag == 0) {
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option, Run '%s  -h' for option "
			 " information.", TCID);
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

			if (setup_test(i, lc)) {
				tst_resm(TWARN, "Not able to test mount(2) for "
					 "error %s as setup failed",
					 testcases[i].exp_errval);
				continue;
			}

			/* Call umount(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.*/

			TEST(umount(Mntpoint));

			/* check return code */
			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases[i].exp_errno)) {
				tst_resm(TPASS, "umount(2) expected failure; "
					 "Got errno - %s : %s",
					 testcases[i].exp_errval,
					 testcases[i].err_desc);
			} else {
				tst_resm(TFAIL, "umount(2) failed to produce "
					 "expected error; %d, errno:%s got %d",
					 testcases[i].exp_errno,
					 testcases[i].exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			cleanup_test(i);
		}
	}

	cleanup();
	tst_exit();
}

static int setup_test(int i, int cnt)
{
	char temp[20];

	Device = device;
	Fstype = fstype;
	Mntpoint = mntpoint;
	Flag = 0;

	switch (i) {
	case 0:
		/* Setup for umount(2) returning errno EBUSY. */
		if (access(Device, F_OK)) {
			tst_brkm(TBROK, cleanup,
				 "Device %s does not exist", Device);
			return 1;
		}

		TEST(mount(Device, Mntpoint, Fstype, Flag, NULL));

		if (TEST_RETURN == -1) {
			tst_brkm(TBROK, cleanup, "mount(2) failed to mount "
				 "device %s at mountpoint %s, Got errno - %d :"
				 " %s", Device, Mntpoint, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			return 1;
		}

		if (getcwd(Path, PATH_MAX) == NULL) {
			tst_resm(TWARN, "getcwd() failed to get current working"
				 " directory errno = %d : %s", errno,
				 strerror(errno));
			return 1;
		}
		sprintf(temp, "/%s/t_%d", Mntpoint, cnt);
		strcat(Path, temp);
		if ((fd = open(Path, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
			tst_resm(TWARN, "open() failed to create a file "
				 " %s errno = %d : %s", Path, errno,
				 strerror(errno));
			return 1;
		} else {
			return 0;
		}
	case 1:

		/* Setup for umount(2) returning errno EFAULT. */

		Mntpoint = NULL;
		break;
	case 2:
		/* Setup for umount(2) returning errno ENOENT. */

		Mntpoint = Einval;
		break;
	case 3:
		/* Setup for umount(2) returning errno EINVAL. */

		if (getcwd(Path, PATH_MAX) == NULL) {
			tst_resm(TWARN, "getcwd() failed to get current working"
				 " directory errno = %d : %s", errno,
				 strerror(errno));
			return 1;
		}
		Mntpoint = Path;
		break;
	case 4:
		/* Setup for umount(2) returning errno ENAMETOOLONG. */

		memset(Longpathname, 'a', PATH_MAX + 2);
		Mntpoint = Longpathname;
		break;
	}
	return 0;
}

void cleanup_test(int i)
{
	switch (i) {
	case 0:
		close(fd);
		TEST(umount(mntpoint));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN, "umount(2) Failed while unmounting"
				 " errno %d for testcase %s", TEST_ERRNO,
				 testcases[i].exp_errval);
		}
		break;
	}
}

static void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);
	tst_mkfs(NULL, device, fstype, NULL);

	tst_tmpdir();

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) failed; "
			 "errno = %d: %s", mntpoint, DIR_MODE, errno,
			 strerror(errno));
	}

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}

static void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
