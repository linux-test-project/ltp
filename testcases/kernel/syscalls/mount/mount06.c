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
#include "usctest.h"
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
static void help(void);

char *TCID = "mount06";
int TST_TOTAL = 1;

static char *fstype = "ext2";

static int dflag;
static char *device;
static char path_name[PATH_MAX];
static char mntpoint_src[PATH_MAX];
static char mntpoint_des[PATH_MAX];

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &dflag, &device},
	{NULL, NULL, NULL},
};

int main(int argc, char *argv[])
{
	int lc;
	char *msg;

	msg = parse_opts(argc, argv, options, &help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (!dflag)
		tst_brkm(TBROK, NULL,
			 "you must specify the device used for mounting with "
			 "the -D option");

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		if (mount(device, mntpoint_src, fstype, 0, NULL) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "mount %s failed",
				 mntpoint_src);

		TEST(mount(mntpoint_src, mntpoint_des, fstype, MS_MOVE, NULL));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL | TTERRNO, "mount(2) failed");
		} else {

			if (!ismount(mntpoint_src) && ismount(mntpoint_des))
				tst_resm(TPASS, "move mount is ok");
			else
				tst_resm(TFAIL, "move mount does not work");

			TEST(umount(mntpoint_des));
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

void setup(void)
{
	tst_require_root(NULL);
	tst_mkfs(NULL, device, fstype, NULL);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	if (getcwd(path_name, sizeof(path_name)) == NULL)
		tst_brkm(TBROK, cleanup, "getcwd failed");

	/*
	 * Turn current dir into a private mount point being a parent
	 * mount which is required by move mount.
	 */
	if (mount(path_name, path_name, "none", MS_BIND, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "bind mount failed");

	if (mount("none", path_name, "none", MS_PRIVATE, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mount private failed");

	snprintf(mntpoint_src, PATH_MAX, "%s/%s", path_name, MNTPOINT_SRC);
	snprintf(mntpoint_des, PATH_MAX, "%s/%s", path_name, MNTPOINT_DES);

	SAFE_MKDIR(cleanup, mntpoint_src, DIR_MODE);
	SAFE_MKDIR(cleanup, mntpoint_des, DIR_MODE);

	TEST_PAUSE;
}

void cleanup(void)
{
	if (umount(path_name) != 0)
		tst_brkm(TBROK | TERRNO, NULL, "umount(2) %s failed",
			 path_name);

	TEST_CLEANUP;

	tst_rmdir();
}

void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted. "
	       "Default ext2.\n");
	printf("-D device : device used for mounting.\n");
}
