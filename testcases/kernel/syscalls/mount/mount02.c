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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/******************************************************************************
 *
 *    TEST IDENTIFIER	: mount02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test for checking basic error conditions  for mount(2)
 *
 *    TEST CASE TOTAL	: 13
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Check for basic errors returned by mount(2) system call.
 *
 *	Verify that mount(2) returns -1 and sets errno to
 *
 *	1) ENODEV if filesystem type not configured
 *	2) ENOTBLK if specialfile is not a block device
 *	3) EBUSY if specialfile is already mounted or
 *		 it  cannot  be remounted read-only, because it still holds
 *		 files open for writing.
 *	4) EINVAL if specialfile or device is invalid or
 *		 a remount was attempted, while source was not already
 *		 mounted on target.
 *	5) EFAULT if specialfile or device file points to invalid address space.
 *	6) ENAMETOOLONG if pathname was longer than MAXPATHLEN.
 *	7) ENOENT if pathname was empty or has a nonexistent component.
 *	8) ENOTDIR if not a directory.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Create a mount point.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Do necessary setup for each test.
 *	  Execute system call
 *	  Check return code, if system call failed and errno == expected errno
 *		Issue sys call passed with expected return value and errno.
 *	  Otherwise,
 *		Issue sys call failed to produce expected error.
 *	  Do cleanup for each test.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *	  Delete the temporary directory(s)/file(s) created.
 *
 * USAGE:  <for command-line>
 *  mount02 [-T type] -D device [-e] [-i n] [-I x] [-p x] [-t]
 *			where,  -T type : specifies the type of filesystem to
 *					  be mounted. Default ext2.
 *				-D device : device to be mounted.
 *				-e   : Turn on errno logging.
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
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
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);

static int setup_test(int, int);
static int cleanup_test(int);

char *TCID = "mount02";		/* Test program identifier.    */

#define DEFAULT_FSTYPE "ext2"
#define FSTYPE_LEN	20
#define DIR_MODE	S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP
#define FILE_MODE	S_IRWXU | S_IRWXG | S_IRWXO

static char *Einval = (char *)-1;
static char Longpathname[PATH_MAX + 2];
static char Path[PATH_MAX];
static char *Type;
static char *Fstype;
static char *Device;
static char *Mntpoint;
static unsigned long Flag;

static int fd;
static char mntpoint[PATH_MAX];
static char *fstype;
static char *device;
static int Tflag = 0;
static int Dflag = 0;

static int exp_enos[] = { ENODEV, ENOTBLK, EBUSY, EINVAL, EFAULT, ENAMETOOLONG,
	ENOENT, ENOTDIR, 0
};

int TST_TOTAL = (sizeof(exp_enos) / sizeof(exp_enos[0])) - 1;

static option_t options[] = {	/* options supported by mount02 test */
	{"T:", &Tflag, &fstype},	/* -T type of filesystem        */
	{"D:", &Dflag, &device},	/* -D device used for mounting  */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (Dflag == 0)
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option.");

	Type = malloc(FSTYPE_LEN);
	if (Type == NULL) {
		tst_brkm(TBROK|TERRNO, NULL, "malloc failed");
	}

	if (Tflag == 1) {
		strncpy(Type, fstype,
			(FSTYPE_LEN <
			 strlen(fstype)) ? FSTYPE_LEN : strlen(fstype));
	} else {
		strncpy(Type, DEFAULT_FSTYPE, strlen(DEFAULT_FSTYPE));
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			if (setup_test(i, lc) != 0) {
				tst_resm(TWARN, "testcase setup failed");
				continue;
			}

			/* Call mount(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.*/

			TEST(mount(Device, Mntpoint, Fstype, Flag, NULL));

			/* check return code */
			if (TEST_RETURN == -1 && TEST_ERRNO == exp_enos[i]) {
				tst_resm(TPASS|TERRNO,
				    "mount got expected failure");
			} else {
				if (umount(mntpoint) == -1) {
					tst_brkm(TBROK|TERRNO, cleanup,
					    "umount of %s failed", Mntpoint);
				}
				tst_resm(TFAIL|TERRNO,
				    "mount(2) failed to produce expected "
				    "error (%d)",
				    exp_enos[i]);
			}

			(void)cleanup_test(i);

		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

/*
 * int
 * setup_test() - Setup function for test cases based on the error values
 *		  to be returned.
 */
int setup_test(int i, int cnt)
{
	char temp[20];

	Device = device;
	Fstype = Type;
	Mntpoint = mntpoint;
	Flag = 0;
	switch (i) {
	case 0:
		/* Setup for mount(2) returning errno ENODEV. */

		strncpy(Type, "error", 5);
		Fstype = Type;
		return 0;
	case 1:
		/* Setup for mount(2) returning errno ENOTBLK. */

		sprintf(Path, "./mydev_%d_%d", getpid(), cnt);
		TEST(mknod(Path, S_IFCHR | FILE_MODE, 0));
		if (TEST_RETURN == 0) {
			Device = Path;
			return 0;
		} else {
			tst_resm(TWARN, "mknod(2) failed to create device: %s",
			    Path);
			return 1;
		}
	case 2:
		/* Setup for mount(2) returning errno EBUSY. */

		TEST(mount(Device, Mntpoint, Fstype, 0, NULL));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN|TTERRNO,
			    "mount(2) failed to mount device %s",
			    device);
			return 1;
		}
		return 0;
	case 3:
		/* Setup for mount(2) returning errno EBUSY. */

		TEST(mount(Device, Mntpoint, Fstype, 0, NULL));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN|TTERRNO,
			    "mount(2) failed to mount device %s", device);
			return 1;
		}
		if (getcwd(Path, PATH_MAX) == NULL) {
			tst_resm(TWARN|TERRNO, "getcwd failed");
			return 1;
		}
		sprintf(temp, "/%s/t3_%d", mntpoint, cnt);
		strcat(Path, temp);
		if ((fd = open(Path, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
			tst_resm(TWARN|TERRNO, "open() failed to create %s",
			    Path);
			return 1;
		}
		Flag = MS_REMOUNT | MS_RDONLY;
		return 0;
	case 4:
		/* Setup for mount(2) returning errno EINVAL. */

		Device = NULL;
		break;
	case 5:
		/* Setup for mount(2) returning errno EINVAL. */

		Fstype = NULL;
		break;
	case 6:
		/* Setup for mount(2) returning errno EINVAL. */

		Flag = MS_REMOUNT;
		break;
	case 7:
		/* Setup for mount(2) returning errno EFAULT. */

		Fstype = Einval;
		break;
	case 8:
		/* Setup for mount(2) returning errno EFAULT. */

		Device = Einval;
		break;
	case 9:
		/* Setup for mount(2) returning errno ENAMETOOLONG. */

		memset(Longpathname, 'a', PATH_MAX + 2);
		Mntpoint = Longpathname;
		break;
	case 10:
		/* Setup for mount(2) returning errno ENOENT. */

		strncpy(Path, "   ", 3);
		Mntpoint = Path;
		break;
	case 11:
		/* Setup for mount(2) returning errno ENOENT. */

		sprintf(Path, "%s/noexistent", mntpoint);
		Mntpoint = Path;
		break;
	case 12:
		/* Setup for mount(2) returning errno ENOTDIR. */

		if (getcwd(Path, PATH_MAX) == NULL) {
			tst_resm(TWARN, "getcwd failed");
			return 1;
		}
		sprintf(temp, "/t_%d_%d", getpid(), cnt);
		strcat(Path, temp);
		if ((fd = open(Path, O_CREAT, S_IRWXU)) == -1) {
			tst_resm(TWARN, "open failed to create %s", Path);
			return 1;
		} else {
			Mntpoint = Path;
			return 0;
		}
	}
	return 0;
}

/*
 * int
 * cleanup_test() - Setup function for test cases based on the error values
 *		  to be returned.
 */
int cleanup_test(int i)
{
	switch (i) {
	case 0:
	case 5:
	case 7:
		if (Tflag) {
			/* Avoid buffer overflow */
			strncpy(Type, fstype,
				(FSTYPE_LEN < strlen(fstype)+1) ? FSTYPE_LEN :
				strlen(fstype)+1);
		} else {
			strcpy(Type, "ext2");
		}
		break;
	case 3:
		close(fd);
		/* FALLTHROUGH */
	case 2:
		TEST(umount(mntpoint));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN|TTERRNO, "umount failed");
		}
		break;
	case 12:
		close(fd);
		break;
	}
	return 0;
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	/* make a temp directory */
	tst_tmpdir();

	(void)sprintf(mntpoint, "mnt_%d", getpid());

	if (mkdir(mntpoint, DIR_MODE) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir(%s, %#o) failed",
		    mntpoint, DIR_MODE);
	}

	/* set up expected error numbers */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	free(Type);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();
}

/*
 * issue a help message
 */
void help()
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}