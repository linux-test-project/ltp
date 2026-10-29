// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that mknod(2) fails with the correct error codes:
 *
 * - EACCES if parent directory does not allow write permission to the process.
 * - EPERM if the process id of the caller is not super-user.
 * - EROFS if pathname refers to a file on a read-only file system.
 * - ELOOP if too many symbolic links were encountered in resolving pathname.
 */

#include <pwd.h>
#include <sys/sysmacros.h>

#include "tst_test.h"

#define TEMP_MNT "mnt"
#define TEMP_DIR "testdir"
#define TEMP_DIR_MODE 0500

#define ELOOP_DIR "test_eloop"
#define ELOOP_FILE "/test_eloop"
#define ELOOP_DIR_MODE 0755
#define ELOOP_MAX 43

#define FIFO_MODE (S_IFIFO | 0444)
#define SOCKET_MODE (S_IFSOCK | 0777)
#define CHR_MODE (S_IFCHR | 0600)
#define BLK_MODE (S_IFBLK | 0600)

static char *elooppathname;

static struct tcase {
	char *pathname;
	int mode;
	int exp_errno;
	int major, minor;
} tcases[] = {
	{ NULL, FIFO_MODE, ELOOP, 0, 0 },
	{ TEMP_DIR "/tnode1", SOCKET_MODE, EACCES, 0, 0 },
	{ TEMP_DIR "/tnode2", FIFO_MODE, EACCES, 0, 0 },
	{ "tnode3", CHR_MODE, EPERM, 1, 3 },
	{ "tnode4", BLK_MODE, EPERM, 0, 0 },
	{ TEMP_MNT "/tnode5", SOCKET_MODE, EROFS, 0, 0 },
};

#define TEST_SIZE ARRAY_SIZE(tcases)

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(mknod(tc->pathname, tc->mode,
			   makedev(tc->major, tc->minor)),
		     tc->exp_errno);
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	SAFE_SETEUID(ltpuser->pw_uid);
	SAFE_MKDIR(TEMP_DIR, TEMP_DIR_MODE);

	SAFE_MKDIR(ELOOP_DIR, ELOOP_DIR_MODE);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");

	/*
	 * The kernel limits symlink resolution hop amount to 40,
	 * create a pathname with more than that
	 */
	strcpy(elooppathname, ".");
	for (int i = 0; i < ELOOP_MAX; i++)
		strcat(elooppathname, ELOOP_FILE);
	tcases[0].pathname = elooppathname;
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.mntpoint = TEMP_MNT,
	.needs_rofs = 1,
	.bufs = (struct tst_buffers[]){
		{ &elooppathname, .size = sizeof(ELOOP_FILE) * ELOOP_MAX },
		{},
	},
};
