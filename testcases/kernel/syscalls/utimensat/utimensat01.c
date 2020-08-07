// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2008 Michael Kerrisk <mtk.manpages@gmail.com>
 * Copyright (c) 2008 Subrata Modak <subrata@linux.vnet.ibm.com>
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic utimnsat() test.
 */

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "lapi/fs.h"
#include "tst_timer.h"

#define UTIME_NOW	((1l << 30) - 1l)
#define UTIME_OMIT	((1l << 30) - 2l)

#define TEST_FILE	"test_file"
#define TEST_DIR	"test_dir"

static void *bad_addr;

struct mytime {
	long access_tv_sec;
	long access_tv_nsec;
	long mod_tv_sec;
	long mod_tv_nsec;
	int atime_change;
	int mtime_change;
};

static struct mytime tnn = {0, UTIME_NOW, 0, UTIME_NOW, 1, 1};
static struct mytime too = {0, UTIME_OMIT, 0, UTIME_OMIT, 0, 0};
static struct mytime tno = {0, UTIME_NOW, 0, UTIME_OMIT, 1, 0};
static struct mytime ton = {0, UTIME_OMIT, 0, UTIME_NOW, 0, 1};
static struct mytime t11 = {1, 1, 1, 1, 1, 1};

struct test_case {
	int dirfd;
	char *pathname;
	struct mytime *mytime;
	int flags;
	int oflags;
	int attr;
	int mode;
	int exp_err;
} tcase[] = {
	/* Testing read-only file */
	{AT_FDCWD, TEST_FILE, NULL, 0, O_RDONLY, 0, 0400, 0},
	{AT_FDCWD, TEST_FILE, &tnn, 0, O_RDONLY, 0, 0400, 0},
	{AT_FDCWD, TEST_FILE, &too, 0, O_RDONLY, 0, 0400, 0},
	{AT_FDCWD, TEST_FILE, &tno, 0, O_RDONLY, 0, 0400, 0},
	{AT_FDCWD, TEST_FILE, &ton, 0, O_RDONLY, 0, 0400, 0},
	{AT_FDCWD, TEST_FILE, &t11, 0, O_RDONLY, 0, 0400, 0},

	/* Testing writable file */
	{AT_FDCWD, TEST_FILE, NULL, 0, O_RDONLY, 0, 0666, 0},
	{AT_FDCWD, TEST_FILE, &tnn, 0, O_RDONLY, 0, 0666, 0},
	{AT_FDCWD, TEST_FILE, &too, 0, O_RDONLY, 0, 0666, 0},
	{AT_FDCWD, TEST_FILE, &tno, 0, O_RDONLY, 0, 0666, 0},
	{AT_FDCWD, TEST_FILE, &ton, 0, O_RDONLY, 0, 0666, 0},
	{AT_FDCWD, TEST_FILE, &t11, 0, O_RDONLY, 0, 0666, 0},

	/* Testing append-only file */
	{AT_FDCWD, TEST_FILE, NULL, 0, O_RDONLY, FS_APPEND_FL, 0600, 0},
	{AT_FDCWD, TEST_FILE, &tnn, 0, O_RDONLY, FS_APPEND_FL, 0600, 0},
	{AT_FDCWD, TEST_FILE, &too, 0, O_RDONLY, FS_APPEND_FL, 0600, 0},
	{AT_FDCWD, TEST_FILE, &tno, 0, O_RDONLY, FS_APPEND_FL, 0600, EPERM},
	{AT_FDCWD, TEST_FILE, &ton, 0, O_RDONLY, FS_APPEND_FL, 0600, EPERM},
	{AT_FDCWD, TEST_FILE, &t11, 0, O_RDONLY, FS_APPEND_FL, 0600, EPERM},

	/* Testing immutable file */
	{AT_FDCWD, TEST_FILE, NULL, 0, O_RDONLY, FS_IMMUTABLE_FL, 0600, -1},
	{AT_FDCWD, TEST_FILE, &tnn, 0, O_RDONLY, FS_IMMUTABLE_FL, 0600, -1},
	{AT_FDCWD, TEST_FILE, &too, 0, O_RDONLY, FS_IMMUTABLE_FL, 0600, 0},
	{AT_FDCWD, TEST_FILE, &tno, 0, O_RDONLY, FS_IMMUTABLE_FL, 0600, EPERM},
	{AT_FDCWD, TEST_FILE, &ton, 0, O_RDONLY, FS_IMMUTABLE_FL, 0600, EPERM},
	{AT_FDCWD, TEST_FILE, &t11, 0, O_RDONLY, FS_IMMUTABLE_FL, 0600, EPERM},

	/* Testing immutable-append-only file */
	{AT_FDCWD, TEST_FILE, NULL, 0, O_RDONLY, FS_APPEND_FL|FS_IMMUTABLE_FL, 0600, -1},
	{AT_FDCWD, TEST_FILE, &tnn, 0, O_RDONLY, FS_APPEND_FL|FS_IMMUTABLE_FL, 0600, -1},
	{AT_FDCWD, TEST_FILE, &too, 0, O_RDONLY, FS_APPEND_FL|FS_IMMUTABLE_FL, 0600, 0},
	{AT_FDCWD, TEST_FILE, &tno, 0, O_RDONLY, FS_APPEND_FL|FS_IMMUTABLE_FL, 0600, EPERM},
	{AT_FDCWD, TEST_FILE, &ton, 0, O_RDONLY, FS_APPEND_FL|FS_IMMUTABLE_FL, 0600, EPERM},
	{AT_FDCWD, TEST_FILE, &t11, 0, O_RDONLY, FS_APPEND_FL|FS_IMMUTABLE_FL, 0600, EPERM},

	/* Other failure tests */
	{AT_FDCWD, TEST_FILE, NULL, 0, O_RDONLY, 0, 0400, EFAULT},
	{AT_FDCWD, NULL, &tnn, 0, O_RDONLY, 0, 0400, EFAULT},
	{-1, NULL, &tnn, AT_SYMLINK_NOFOLLOW, O_RDONLY, 0, 0400, EINVAL},
	{-1, TEST_FILE, &tnn, 0, O_RDONLY, 0, 0400, ENOENT},
};

static inline int sys_utimensat(int dirfd, const char *pathname,
			const struct __kernel_old_timespec times[2], int flags)
{
	return tst_syscall(__NR_utimensat, dirfd, pathname, times, flags);
}

static void update_error(struct test_case *tc)
{
	if (tc->exp_err != -1)
		return;

	/*
	 * Starting with 4.8.0 operations on immutable files return EPERM
	 * instead of EACCES.
	 * This patch has also been merged to stable 4.4 with
	 * b3b4283 ("vfs: move permission checking into notify_change() for utimes(NULL)")
	 */
	if (tst_kvercmp(4, 4, 27) < 0)
		tc->exp_err = EACCES;
	else
		tc->exp_err = EPERM;
}

static void change_attr(struct test_case *tc, int fd, int set)
{
	int attr;

	if (!tc->attr)
		return;

	SAFE_IOCTL(fd, FS_IOC_GETFLAGS, &attr);

	if (set)
		attr |= tc->attr;
	else
		attr &= ~tc->attr;

	SAFE_IOCTL(fd, FS_IOC_SETFLAGS, &attr);
}

static void reset_time(char *pathname, int dfd, int flags, int i)
{
	struct __kernel_old_timespec ts[2];
	struct stat sb;

	memset(ts, 0, sizeof(ts));
	sys_utimensat(dfd, pathname, ts, flags);

	TEST(stat(pathname, &sb));
	if (TST_RET) {
		tst_res(TFAIL | TTERRNO, "%2d: stat() failed", i);
	} else if (sb.st_atime || sb.st_mtime) {
		tst_res(TFAIL, "Failed to reset access and modification time (%lu: %lu)",
			sb.st_atime, sb.st_mtime);
	}
}

static void run(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	struct __kernel_old_timespec ts[2];
	int dfd = AT_FDCWD, fd = 0, atime_change, mtime_change;
	struct mytime *mytime = tc->mytime;
	char *pathname = NULL;
	void *tsp = NULL;
	struct stat sb;

	if (tc->dirfd != AT_FDCWD)
		dfd = SAFE_OPEN(TEST_DIR, tc->oflags);

	if (tc->pathname) {
		fd = SAFE_OPEN(tc->pathname, O_WRONLY | O_CREAT);
		pathname = tc->pathname;
		SAFE_CHMOD(tc->pathname, tc->mode);
		reset_time(pathname, dfd, tc->flags, i);
		change_attr(tc, fd, 1);
	} else if (tc->exp_err == EFAULT) {
		pathname = bad_addr;
	}

	if (mytime) {
		ts[0].tv_sec = mytime->access_tv_sec;
		ts[0].tv_nsec = mytime->access_tv_nsec;
		ts[1].tv_sec = mytime->mod_tv_sec;
		ts[1].tv_nsec = mytime->mod_tv_nsec;
		tsp = ts;
	} else if (tc->exp_err == EFAULT) {
		tsp = bad_addr;
	}

	TEST(sys_utimensat(dfd, pathname, tsp, tc->flags));
	if (tc->pathname)
		change_attr(tc, fd, 0);

	if (TST_RET) {
		if (!tc->exp_err) {
			tst_res(TFAIL | TTERRNO, "%2d: utimensat() failed", i);
		} else if (tc->exp_err == TST_ERR) {
			tst_res(TPASS | TTERRNO, "%2d: utimensat() failed expectedly", i);
		} else {
			tst_res(TFAIL | TTERRNO, "%2d: utimensat() failed with incorrect error, expected %s",
				i, tst_strerrno(tc->exp_err));
		}
	} else if (tc->exp_err) {
		tst_res(TFAIL, "%2d: utimensat() passed unexpectedly", i);
	} else {
		atime_change = mytime ? mytime->atime_change : 1;
		mtime_change = mytime ? mytime->mtime_change : 1;

		TEST(stat(tc->pathname ? tc->pathname : TEST_DIR, &sb));
		if (TST_RET) {
			tst_res(TFAIL | TTERRNO, "%2d: stat() failed", i);
			goto close;
		}

		if (!!sb.st_atime != atime_change) {
			tst_res(TFAIL, "%2d: atime %s have changed but %s",
				i, atime_change ? "should" : "shouldn't",
				sb.st_atime ? "did" : "didn't");
		} else if (!!sb.st_mtime != mtime_change) {
			tst_res(TFAIL, "%2d: mtime %s have changed but %s",
				i, mtime_change ? "should" : "shouldn't",
				sb.st_mtime ? "did" : "didn't");
		} else {
			tst_res(TPASS, "%2d: utimensat() passed", i);
		}
	}

close:
	if (dfd != AT_FDCWD)
		SAFE_CLOSE(dfd);

	if (tc->pathname)
		SAFE_CLOSE(fd);
}

static void setup(void)
{
	size_t i;

	bad_addr = tst_get_bad_addr(NULL);
	SAFE_MKDIR(TEST_DIR, 0700);

	for (i = 0; i < ARRAY_SIZE(tcase); i++)
		update_error(&tcase[i]);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcase),
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
