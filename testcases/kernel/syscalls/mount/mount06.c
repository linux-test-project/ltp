/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: DAN LI <li.dan@cn.fujitsu.com>
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
 */

/*
 *  DESCRIPTION
 *	Test for feature MS_MOVE of mount(2).
 *	"Move an existing mount point to the new location."
 */

#include <errno.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>

#include "test.h"
#include "safe_macros.h"

#ifndef MS_MOVE
#define MS_MOVE	8192
#endif

#ifndef MS_PRIVATE
#define MS_PRIVATE	(1 << 18)
#endif

#define MNTPOINT_SRC	"mnt_src"
#define MNTPOINT_DES	"mnt_des"
#define LINELENGTH	256
#define DIR_MODE	(S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP)

static int ismount(char *mntpoint);
static void setup(void);
static void cleanup(void);

char *TCID = "mount06";
int TST_TOTAL = 1;

static const char *fs_type;
static const char *device;
static char path_name[PATH_MAX];
static char mntpoint_src[PATH_MAX];
static char mntpoint_des[PATH_MAX];
static int mount_flag;

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		SAFE_MOUNT(cleanup, device, mntpoint_src, fs_type, 0, NULL);

		TEST(mount(mntpoint_src, mntpoint_des, fs_type, MS_MOVE, NULL));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL | TTERRNO, "mount(2) failed");
		} else {

			if (!ismount(mntpoint_src) && ismount(mntpoint_des))
				tst_resm(TPASS, "move mount is ok");
			else
				tst_resm(TFAIL, "move mount does not work");

			TEST(tst_umount(mntpoint_des));
			if (TEST_RETURN != 0)
				tst_brkm(TBROK | TTERRNO, cleanup,
					 "umount(2) failed");
		}
	}

	cleanup();
	tst_exit();
}

int ismount(char *mntpoint)
{
	int ret = 0;
	FILE *file;
	char line[LINELENGTH];

	file = fopen("/proc/mounts", "r");
	if (file == NULL)
		tst_brkm(TFAIL | TERRNO, NULL, "Open /proc/mounts failed");

	while (fgets(line, LINELENGTH, file) != NULL) {
		if (strstr(line, mntpoint) != NULL) {
			ret = 1;
			break;
		}
	}
	fclose(file);
	return ret;
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	if (getcwd(path_name, sizeof(path_name)) == NULL)
		tst_brkm(TBROK, cleanup, "getcwd failed");

	/*
	 * Turn current dir into a private mount point being a parent
	 * mount which is required by move mount.
	 */
	SAFE_MOUNT(cleanup, path_name, path_name, "none", MS_BIND, NULL);

	mount_flag = 1;

	SAFE_MOUNT(cleanup, "none", path_name, "none", MS_PRIVATE, NULL);

	snprintf(mntpoint_src, PATH_MAX, "%s/%s", path_name, MNTPOINT_SRC);
	snprintf(mntpoint_des, PATH_MAX, "%s/%s", path_name, MNTPOINT_DES);

	SAFE_MKDIR(cleanup, mntpoint_src, DIR_MODE);
	SAFE_MKDIR(cleanup, mntpoint_des, DIR_MODE);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(path_name) != 0)
		tst_resm(TWARN | TERRNO, "umount(2) %s failed", path_name);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
