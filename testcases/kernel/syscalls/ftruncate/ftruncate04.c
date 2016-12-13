/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 *  Robbie Williamson <robbiew@us.ibm.com>
 *  Roy Lee <roylee@andestech.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Tests truncate and mandatory record locking.
 *
 * Parent creates a file, child locks a region and sleeps.
 *
 * Parent checks that ftruncate before the locked region and inside the region
 * fails while ftruncate after the region succeds.
 *
 * Parent wakes up child, child exits, lock is unlocked.
 *
 * Parent checks that ftruncate now works in all cases.
 */

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/statvfs.h>

#include "test.h"
#include "safe_macros.h"

#define RECLEN	100
#define MOUNT_DIR "dir/"

const char *TCID = "ftruncate04";
int TST_TOTAL = 6;

static int len = 8 * 1024;
static char filename[80];

static int recstart;
static int reclen;

static const char *device;
static const char *fs_type;
static int mount_flag;

static void dochild(void);
static void doparent(void);
static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc, pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

#ifdef UCLINUX
	maybe_run_child(&dochild, "sdd", filename, &recstart, &reclen);
#endif

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		sprintf(filename, MOUNT_DIR"%s.%d.%d\n", TCID, getpid(), lc);

		if (tst_fill_file(filename, 0, 1024, 8)) {
			tst_brkm(TBROK, cleanup,
			         "Failed to create test file '%s'", filename);
		}

		SAFE_CHMOD(cleanup, filename, 02666);

		reclen = RECLEN;
		/*
		 * want at least RECLEN bytes BEFORE AND AFTER the
		 * record lock.
		 */
		recstart = RECLEN + rand() % (len - 3 * RECLEN);

		if ((pid = FORK_OR_VFORK()) < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");

		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "sdd", filename, recstart,
			              reclen) < -1) {
				tst_brkm(TBROK, cleanup, "self_exec() failed");
			}
#else
			dochild();
#endif
		}

		doparent();
	}

	cleanup();
	tst_exit();
}

static void ftruncate_expect_fail(int fd, off_t offset, const char *msg)
{
	TEST(ftruncate(fd, offset));

	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "ftruncate() %s succeeded unexpectedly", msg);
		return;
	}

	if (TEST_ERRNO != EAGAIN) {
		tst_resm(TFAIL | TTERRNO,
		         "ftruncate() %s failed unexpectedly, expected EAGAIN",
			 msg);
		return;
	}

	tst_resm(TPASS, "ftruncate() %s failed with EAGAIN", msg);
}

static void ftruncate_expect_success(int fd, off_t offset, const char *msg)
{
	struct stat sb;

	TEST(ftruncate(fd, offset));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO,
		         "ftruncate() %s failed unexpectedly", msg);
		return;
	}

	SAFE_FSTAT(cleanup, fd, &sb);

	if (sb.st_size != offset) {
		tst_resm(TFAIL,
			 "ftruncate() to %zu bytes succeded but fstat() reports size %zu",
			 offset, sb.st_size);
		return;
	}

	tst_resm(TPASS, "ftruncate() %s succeded", msg);
}

static void doparent(void)
{
	int fd;

	/* Wait for child lock */
	TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);

	fd = SAFE_OPEN(cleanup, filename, O_RDWR | O_NONBLOCK);

	ftruncate_expect_fail(fd, RECLEN, "offset before lock");
	ftruncate_expect_fail(fd, recstart + RECLEN/2, "offset in lock");
	ftruncate_expect_success(fd, recstart + RECLEN, "offset after lock");

	/* wake child and wait for it to exit (to free record lock) */
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
	SAFE_WAIT(NULL, NULL);

	ftruncate_expect_success(fd, recstart + RECLEN/2, "offset in lock");
	ftruncate_expect_success(fd, recstart, "offset before lock");
	ftruncate_expect_success(fd, recstart + RECLEN, "offset after lock");

	SAFE_CLOSE(NULL, fd);
}

void dochild(void)
{
	int fd;
	struct flock flocks;

#ifdef UCLINUX
	TST_CHECKPOINT_INIT(NULL);
#endif

	fd = SAFE_OPEN(NULL, filename, O_RDWR);

	tst_resm(TINFO, "Child locks file");

	flocks.l_type = F_WRLCK;
	flocks.l_whence = SEEK_CUR;
	flocks.l_start = recstart;
	flocks.l_len = reclen;

	if (fcntl(fd, F_SETLKW, &flocks) < 0)
		tst_brkm(TFAIL, NULL, "child fcntl failed");

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	tst_resm(TINFO, "Child unlocks file");

	tst_exit();
}

static void setup(void)
{
	struct statvfs fs;

	srand(getpid());

	tst_tmpdir();

	SAFE_MKDIR(tst_rmdir, MOUNT_DIR, 0777);

	TST_CHECKPOINT_INIT(tst_rmdir);

	if (statvfs(".", &fs) == -1)
		tst_brkm(TFAIL | TERRNO, tst_rmdir, "statvfs failed");

	if ((fs.f_flag & MS_MANDLOCK))
		return;

	tst_resm(TINFO, "TMPDIR does not support mandatory locks");

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	/* the kernel returns EPERM when CONFIG_MANDATORY_FILE_LOCKING is not
	 * supported - to avoid false negatives, mount the fs first without
	 * flags and then remount it as MS_MANDLOCK */

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);
	SAFE_MOUNT(cleanup, device, MOUNT_DIR, fs_type, 0, NULL);
	mount_flag = 1;

	if (mount(NULL, MOUNT_DIR, NULL, MS_REMOUNT|MS_MANDLOCK, NULL) == -1) {
		if (errno == EPERM) {
			tst_brkm(TCONF, cleanup, "Mandatory locking (likely) "
				 "not supported by this system");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "Remount with MS_MANDLOCK failed");
		}
	}
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MOUNT_DIR))
		tst_resm(TWARN | TERRNO, "umount(%s) failed", device);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
