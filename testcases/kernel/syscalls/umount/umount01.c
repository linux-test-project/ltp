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
 * AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 * DESCRIPTION
 *	This is a Phase I test for the umount(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * RESTRICTIONS
 *	test must run with the -D option
 *	test doesn't support -c option to run it in parallel, as mount
 *	syscall is not supposed to run in parallel.
 *****************************************************************************/

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);

char *TCID = "umount01";
int TST_TOTAL = 1;

#define DEFAULT_FSTYPE	"ext2"
#define DIR_MODE	S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

static char *mntpoint = "mntpoint";
static char *fstype = "ext2";
static char *device;
static int Dflag = 0;

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &Dflag, &device},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	char *msg;

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

		TEST(mount(device, mntpoint, fstype, 0, NULL));

		if (TEST_RETURN != 0) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_brkm(TBROK, cleanup, "mount(2) Failed errno = %d :"
				 "%s ", TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			TEST(umount(mntpoint));
			if (TEST_RETURN != 0) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "umount(2) Failed while "
					 " unmounting %s errno = %d : %s",
					 mntpoint, TEST_ERRNO,
					 strerror(TEST_ERRNO));
				/* Exit here, otherwise subsequent test fails */
				tst_exit();
			} else {
				tst_resm(TPASS, "umount(2) Passed ");
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);
	tst_mkfs(NULL, device, fstype, NULL);

	tst_tmpdir();

	if (mkdir(mntpoint, DIR_MODE) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) failed; "
			 "errno = %d: %s", mntpoint, DIR_MODE, errno,
			 strerror(errno));
	}

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
