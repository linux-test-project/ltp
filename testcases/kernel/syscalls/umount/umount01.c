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
 *****************************************************************************/

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"

static void setup(void);
static void cleanup(void);

char *TCID = "umount01";
int TST_TOTAL = 1;

#define DEFAULT_FSTYPE	"ext2"
#define DIR_MODE	S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

static const char *mntpoint = "mntpoint";
static const char *fs_type;
static const char *device;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(mount(device, mntpoint, fs_type, 0, NULL));

		if (TEST_RETURN != 0) {
			tst_brkm(TBROK, cleanup, "mount(2) Failed errno = %d :"
				 "%s ", TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			TEST(umount(mntpoint));

			if (TEST_RETURN != 0 && TEST_ERRNO == EBUSY) {
				tst_resm(TINFO, "umount() failed with EBUSY "
				         "possibly some daemon (gvfsd-trash) "
				         "is probing newly mounted dirs");
			}

			if (TEST_RETURN != 0) {
				tst_brkm(TFAIL, NULL, "umount(2) Failed while "
					 " unmounting %s errno = %d : %s",
					 mntpoint, TEST_ERRNO,
					 strerror(TEST_ERRNO));
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

	tst_require_root();

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL);

	if (mkdir(mntpoint, DIR_MODE) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) failed; "
			 "errno = %d: %s", mntpoint, DIR_MODE, errno,
			 strerror(errno));
	}

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (device)
		tst_release_device(NULL, device);

	tst_rmdir();
}
