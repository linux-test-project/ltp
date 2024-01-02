// SPDX-License-Identifier: GPL-2.0-or-later

#include <dirent.h>
#include <errno.h>
#include <sys/mount.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_fs.h"

#define TST_FS_SETUP_OVERLAYFS_MSG "overlayfs is not configured in this kernel"
#define TST_FS_SETUP_OVERLAYFS_CONFIG "lowerdir="OVL_LOWER",upperdir="OVL_UPPER",workdir="OVL_WORK

void tst_create_overlay_dirs(void)
{
	DIR *dir = opendir(OVL_LOWER);

	if (dir == NULL) {
		SAFE_MKDIR(OVL_LOWER, 0755);
		SAFE_MKDIR(OVL_UPPER, 0755);
		SAFE_MKDIR(OVL_WORK, 0755);
		SAFE_MKDIR(OVL_MNT, 0755);
		return;
	}
	closedir(dir);
}

int tst_mount_overlay(const char *file, const int lineno, int strict)
{
	int ret;

	tst_create_overlay_dirs();
	ret = mount("overlay", OVL_MNT, "overlay", 0,
				TST_FS_SETUP_OVERLAYFS_CONFIG);
	if (ret == 0)
		return 0;

	if (errno == ENODEV) {
		if (strict) {
			tst_brk_(file, lineno, TCONF,
				TST_FS_SETUP_OVERLAYFS_MSG);
		} else {
			tst_res_(file, lineno, TINFO,
				TST_FS_SETUP_OVERLAYFS_MSG);
		}
	} else if (strict) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"overlayfs mount failed");
	}
	return ret;
}
