// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2008 Michael Kerrisk <mtk.manpages@gmail.com>
 * Copyright (c) 2008 Subrata Modak <subrata@linux.vnet.ibm.com>
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic utimnsat() test.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "lapi/fs.h"
#include "lapi/utime.h"
#include "time64_variants.h"
#include "tst_timer.h"

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/test_file"
#define TEST_DIR	MNTPOINT"/test_dir"

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

static struct test_case {
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
				void *times, int flags)
{
	return tst_syscall(__NR_utimensat, dirfd, pathname, times, flags);
}

static inline int sys_utimensat_time64(int dirfd, const char *pathname,
				       void *times, int flags)
{
	return tst_syscall(__NR_utimensat_time64, dirfd, pathname, times, flags);
}

static struct time64_variants variants[] = {
#if (__NR_utimensat != __LTP__NR_INVALID_SYSCALL)
	{ .utimensat = sys_utimensat, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_utimensat_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .utimensat = sys_utimensat_time64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static union tst_multi {
	struct timespec libc_ts[2];
	struct __kernel_old_timespec kern_old_ts[2];
	struct __kernel_timespec kern_ts[2];
} ts;

static void multi_set_time(enum tst_ts_type type, struct mytime *mytime)
{
	switch (type) {
	case TST_LIBC_TIMESPEC:
		ts.libc_ts[0].tv_sec = mytime->access_tv_sec;
		ts.libc_ts[0].tv_nsec = mytime->access_tv_nsec;
		ts.libc_ts[1].tv_sec = mytime->mod_tv_sec;
		ts.libc_ts[1].tv_nsec = mytime->mod_tv_nsec;
	break;
	case TST_KERN_OLD_TIMESPEC:
		ts.kern_old_ts[0].tv_sec = mytime->access_tv_sec;
		ts.kern_old_ts[0].tv_nsec = mytime->access_tv_nsec;
		ts.kern_old_ts[1].tv_sec = mytime->mod_tv_sec;
		ts.kern_old_ts[1].tv_nsec = mytime->mod_tv_nsec;
	break;
	case TST_KERN_TIMESPEC:
		ts.kern_ts[0].tv_sec = mytime->access_tv_sec;
		ts.kern_ts[0].tv_nsec = mytime->access_tv_nsec;
		ts.kern_ts[1].tv_sec = mytime->mod_tv_sec;
		ts.kern_ts[1].tv_nsec = mytime->mod_tv_nsec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", type);
	}
}

static void update_error(struct test_case *tc)
{
	static struct tst_kern_exv kvers[] = {
		/* Ubuntu kernel has patch b3b4283 since 4.4.0-48.69 */
		{ "UBUNTU", "4.4.0-48.69" },
		{ NULL, NULL},
	};

	if (tc->exp_err != -1)
		return;

	/*
	 * Starting with 4.8.0 operations on immutable files return EPERM
	 * instead of EACCES.
	 * This patch has also been merged to stable 4.4 with
	 * b3b4283 ("vfs: move permission checking into notify_change() for utimes(NULL)")
	 */
	if (tst_kvercmp2(4, 4, 27, kvers) < 0)
		tc->exp_err = EACCES;
	else
		tc->exp_err = EPERM;
}

static void change_attr(struct test_case *tc, int fd, int set)
{
	int attr;

	if (!tc->attr)
		return;

	if (ioctl(fd, FS_IOC_GETFLAGS, &attr)) {
		if (errno == ENOTTY)
			tst_brk(TCONF | TERRNO, "Attributes not supported by FS");
		else
			tst_brk(TBROK | TERRNO, "ioctl(fd, FS_IOC_GETFLAGS, &attr) failed");
	}

	if (set)
		attr |= tc->attr;
	else
		attr &= ~tc->attr;

	SAFE_IOCTL(fd, FS_IOC_SETFLAGS, &attr);
}

static void reset_time(char *pathname, int dfd, int flags, int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct stat sb;

	memset(&ts, 0, sizeof(ts));
	TEST(tv->utimensat(dfd, pathname, &ts, flags));
	if (TST_RET) {
		tst_res(TINFO | TTERRNO, "%2d: utimensat(%d, %s, {0, 0}, %d) failed",
			i, dfd, pathname, flags);
	}

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
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];
	int dfd = AT_FDCWD, fd = 0, atime_change, mtime_change;
	struct mytime *mytime = tc->mytime;
	char *pathname = NULL;
	void *tsp = NULL;
	struct stat sb;

	if (tc->dirfd != AT_FDCWD)
		dfd = SAFE_OPEN(TEST_DIR, tc->oflags);

	if (tc->pathname) {
		fd = SAFE_OPEN(tc->pathname, O_WRONLY | O_CREAT, 0200);
		pathname = tc->pathname;
		SAFE_CHMOD(tc->pathname, tc->mode);
		reset_time(pathname, dfd, tc->flags, i);
		change_attr(tc, fd, 1);
	} else if (tc->exp_err == EFAULT) {
		pathname = bad_addr;
	}

	if (mytime) {
		multi_set_time(tv->ts_type, mytime);
		tsp = &ts;
	} else if (tc->exp_err == EFAULT) {
		tsp = bad_addr;
	}

	TEST(tv->utimensat(dfd, pathname, tsp, tc->flags));
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

	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);

	bad_addr = tst_get_bad_addr(NULL);
	if (access(TEST_DIR, R_OK))
		SAFE_MKDIR(TEST_DIR, 0700);

	for (i = 0; i < ARRAY_SIZE(tcase); i++)
		update_error(&tcase[i]);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcase),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
};
