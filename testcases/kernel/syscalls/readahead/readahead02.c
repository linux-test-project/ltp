// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 */

/*
 * functional test for readahead() syscall
 *
 * This test is measuring effects of readahead syscall.
 * It mmaps/reads a test file with and without prior call to readahead.
 *
 * The overlay part of the test is regression for:
 *  b833a3660394
 *  ("ovl: add ovl_fadvise()")
 * Introduced by:
 *  5b910bd615ba
 *  ("ovl: fix GPF in swapfile_activate of file from overlayfs over xfs")
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "tst_test.h"
#include "tst_timer.h"
#include "lapi/syscalls.h"

static char testfile[PATH_MAX] = "testfile";
#define DROP_CACHES_FNAME "/proc/sys/vm/drop_caches"
#define MEMINFO_FNAME "/proc/meminfo"
#define PROC_IO_FNAME "/proc/self/io"
#define DEFAULT_FILESIZE (64 * 1024 * 1024)

static size_t testfile_size = DEFAULT_FILESIZE;
static char *opt_fsizestr;
static int pagesize;
static unsigned long cached_max;
static int ovl_mounted;
static int readahead_length  = 4096;
static char sys_bdi_ra_path[PATH_MAX];
static int orig_bdi_limit;

static const char mntpoint[] = OVL_BASE_MNTPOINT;

static int libc_readahead(int fd, off_t offset, size_t len)
{
	return readahead(fd, offset, len);
}

static int fadvise_willneed(int fd, off_t offset, size_t len)
{
	/* Should have the same effect as readahead() syscall */
	errno = posix_fadvise(fd, offset, len, POSIX_FADV_WILLNEED);
	/* posix_fadvise returns error number (not in errno) */
	return errno ? -1 : 0;
}

static struct tcase {
	const char *tname;
	int use_overlay;
	int use_fadvise;
	/* Use either readahead() syscall or POSIX_FADV_WILLNEED */
	int (*readahead)(int, off_t, size_t);
} tcases[] = {
	{ "readahead on file", 0, 0, libc_readahead },
	{ "readahead on overlayfs file", 1, 0, libc_readahead },
	{ "POSIX_FADV_WILLNEED on file", 0, 1, fadvise_willneed },
	{ "POSIX_FADV_WILLNEED on overlayfs file", 1, 1, fadvise_willneed },
};

static int readahead_supported = 1;
static int fadvise_supported = 1;

static int has_file(const char *fname, int required)
{
	struct stat buf;

	if (stat(fname, &buf) == -1) {
		if (errno != ENOENT)
			tst_brk(TBROK | TERRNO, "stat %s", fname);
		if (required)
			tst_brk(TCONF, "%s not available", fname);
		return 0;
	}
	return 1;
}

static void drop_caches(void)
{
	SAFE_FILE_PRINTF(DROP_CACHES_FNAME, "1");
}

static unsigned long get_bytes_read(void)
{
	unsigned long ret;

	SAFE_FILE_LINES_SCANF(PROC_IO_FNAME, "read_bytes: %lu", &ret);

	return ret;
}

static unsigned long get_cached_size(void)
{
	unsigned long ret;

	SAFE_FILE_LINES_SCANF(MEMINFO_FNAME, "Cached: %lu", &ret);

	return ret;
}

static void create_testfile(int use_overlay)
{
	int fd;
	char *tmp;
	size_t i;

	sprintf(testfile, "%s/testfile",
		use_overlay ? OVL_MNT : OVL_BASE_MNTPOINT);
	tst_res(TINFO, "creating test file of size: %zu", testfile_size);
	tmp = SAFE_MALLOC(pagesize);

	/* round to page size */
	testfile_size = testfile_size & ~((long)pagesize - 1);

	fd = SAFE_CREAT(testfile, 0644);
	for (i = 0; i < testfile_size; i += pagesize)
		SAFE_WRITE(1, fd, tmp, pagesize);
	SAFE_FSYNC(fd);
	SAFE_CLOSE(fd);
	free(tmp);
}

/* read_testfile - mmap testfile and read every page.
 * This functions measures how many I/O and time it takes to fully
 * read contents of test file.
 *
 * @do_readahead: call readahead prior to reading file content?
 * @fname: name of file to test
 * @fsize: how many bytes to read/mmap
 * @read_bytes: returns difference of bytes read, parsed from /proc/<pid>/io
 * @usec: returns how many microsecond it took to go over fsize bytes
 * @cached: returns cached kB from /proc/meminfo
 */
static int read_testfile(struct tcase *tc, int do_readahead,
			 const char *fname, size_t fsize,
			 unsigned long *read_bytes, long long *usec,
			 unsigned long *cached)
{
	int fd;
	size_t i = 0;
	long read_bytes_start;
	unsigned char *p, tmp;
	off_t offset = 0;

	fd = SAFE_OPEN(fname, O_RDONLY);

	if (do_readahead) {
		do {
			TEST(tc->readahead(fd, offset, fsize - offset));
			if (TST_RET != 0) {
				SAFE_CLOSE(fd);
				return TST_ERR;
			}

			i++;
			offset += readahead_length;
		} while ((size_t)offset < fsize);
		tst_res(TINFO, "readahead calls made: %zu", i);
		*cached = get_cached_size();

		/* offset of file shouldn't change after readahead */
		offset = SAFE_LSEEK(fd, 0, SEEK_CUR);
		if (offset == 0)
			tst_res(TPASS, "offset is still at 0 as expected");
		else
			tst_res(TFAIL, "offset has changed to: %lu", offset);
	}

	tst_timer_start(CLOCK_MONOTONIC);
	read_bytes_start = get_bytes_read();

	p = SAFE_MMAP(NULL, fsize, PROT_READ, MAP_SHARED | MAP_POPULATE, fd, 0);

	/* for old kernels, where MAP_POPULATE doesn't work, touch each page */
	tmp = 0;
	for (i = 0; i < fsize; i += pagesize)
		tmp = tmp ^ p[i];
	/* prevent gcc from optimizing out loop above */
	if (tmp != 0)
		tst_brk(TBROK, "This line should not be reached");

	if (!do_readahead)
		*cached = get_cached_size();

	SAFE_MUNMAP(p, fsize);

	*read_bytes = get_bytes_read() - read_bytes_start;

	tst_timer_stop();
	*usec = tst_timer_elapsed_us();

	SAFE_CLOSE(fd);
	return 0;
}

static void test_readahead(unsigned int n)
{
	unsigned long read_bytes, read_bytes_ra;
	long long usec, usec_ra;
	unsigned long cached_high, cached_low, cached, cached_ra;
	int ret;
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (tc->use_overlay && !ovl_mounted) {
		tst_res(TCONF,
		        "overlayfs is not configured in this kernel.");
		return;
	}

	create_testfile(tc->use_overlay);

	/* find out how much can cache hold if we read whole file */
	read_testfile(tc, 0, testfile, testfile_size, &read_bytes, &usec,
		      &cached);
	cached_high = get_cached_size();
	sync();
	drop_caches();
	cached_low = get_cached_size();
	cached_max = MAX(cached_max, cached_high - cached_low);

	tst_res(TINFO, "read_testfile(0)");
	read_testfile(tc, 0, testfile, testfile_size, &read_bytes, &usec,
		      &cached);
	if (cached > cached_low)
		cached = cached - cached_low;
	else
		cached = 0;

	sync();
	drop_caches();
	cached_low = get_cached_size();
	tst_res(TINFO, "read_testfile(1)");
	ret = read_testfile(tc, 1, testfile, testfile_size, &read_bytes_ra,
		            &usec_ra, &cached_ra);

	if (ret == EINVAL) {
		if (tc->use_fadvise &&
		    (!tc->use_overlay || !fadvise_supported)) {
			fadvise_supported = 0;
			tst_res(TCONF, "CONFIG_ADVISE_SYSCALLS not configured "
				"in kernel?");
			return;
		}

		if (!tc->use_overlay || !readahead_supported) {
			readahead_supported = 0;
			tst_res(TCONF, "readahead not supported on %s",
				tst_device->fs_type);
			return;
		}
	}

	if (ret) {
		tst_res(TFAIL | TTERRNO, "%s failed on %s",
			tc->use_fadvise ? "fadvise" : "readahead",
			tc->use_overlay ? "overlayfs" :
			tst_device->fs_type);
		return;
	}

	if (cached_ra > cached_low)
		cached_ra = cached_ra - cached_low;
	else
		cached_ra = 0;

	tst_res(TINFO, "read_testfile(0) took: %lli usec", usec);
	tst_res(TINFO, "read_testfile(1) took: %lli usec", usec_ra);
	if (has_file(PROC_IO_FNAME, 0)) {
		tst_res(TINFO, "read_testfile(0) read: %ld bytes", read_bytes);
		tst_res(TINFO, "read_testfile(1) read: %ld bytes",
			read_bytes_ra);
		/* actual number of read bytes depends on total RAM */
		if (read_bytes_ra < read_bytes)
			tst_res(TPASS, "readahead saved some I/O");
		else
			tst_res(TFAIL, "readahead failed to save any I/O");
	} else {
		tst_res(TCONF, "Your system doesn't have /proc/self/io,"
			" unable to determine read bytes during test");
	}

	tst_res(TINFO, "cache can hold at least: %ld kB", cached_max);
	tst_res(TINFO, "read_testfile(0) used cache: %ld kB", cached);
	tst_res(TINFO, "read_testfile(1) used cache: %ld kB", cached_ra);

	if (cached_max * 1024 >= testfile_size) {
		/*
		 * if cache can hold ~testfile_size then cache increase
		 * for readahead should be at least testfile_size/2
		 */
		if (cached_ra * 1024 > testfile_size / 2)
			tst_res(TPASS, "using cache as expected");
		else if (!cached_ra)
			tst_res(TFAIL, "readahead failed to use any cache");
		else
			tst_res(TWARN, "using less cache than expected");
	} else {
		tst_res(TCONF, "Page cache on your system is too small "
			"to hold whole testfile.");
	}
}


/*
 * We try raising bdi readahead limit as much as we can. We write
 * and read back "read_ahead_kb" sysfs value, starting with filesize.
 * If that fails, we try again with lower value.
 * readahead_length used in the test is then set to MIN(bdi limit, 2M),
 * to respect kernels prior to commit 600e19afc5f8a6c.
 */
static void setup_readahead_length(void)
{
	struct stat sbuf;
	char tmp[PATH_MAX], *backing_dev;
	int ra_new_limit, ra_limit;

	/* Find out backing device name */
	SAFE_LSTAT(tst_device->dev, &sbuf);
	if (S_ISLNK(sbuf.st_mode))
		SAFE_READLINK(tst_device->dev, tmp, PATH_MAX);
	else
		strcpy(tmp, tst_device->dev);

	backing_dev = basename(tmp);
	sprintf(sys_bdi_ra_path, "/sys/class/block/%s/bdi/read_ahead_kb",
		backing_dev);
	if (access(sys_bdi_ra_path, F_OK))
		return;

	SAFE_FILE_SCANF(sys_bdi_ra_path, "%d", &orig_bdi_limit);

	/* raise bdi limit as much as kernel allows */
	ra_new_limit = testfile_size / 1024;
	while (ra_new_limit > pagesize / 1024) {
		FILE_PRINTF(sys_bdi_ra_path, "%d", ra_new_limit);
		SAFE_FILE_SCANF(sys_bdi_ra_path, "%d", &ra_limit);

		if (ra_limit == ra_new_limit) {
			readahead_length = MIN(ra_new_limit * 1024,
				2 * 1024 * 1024);
			break;
		}
		ra_new_limit = ra_new_limit / 2;
	}
}

static void setup(void)
{
	if (opt_fsizestr) {
		testfile_size = SAFE_STRTOL(opt_fsizestr, 1, INT_MAX);
		tst_set_max_runtime(1 + testfile_size / (DEFAULT_FILESIZE/32));
	}

	if (access(PROC_IO_FNAME, F_OK))
		tst_brk(TCONF, "Requires " PROC_IO_FNAME);

	has_file(DROP_CACHES_FNAME, 1);
	has_file(MEMINFO_FNAME, 1);

	/* check if readahead is supported */
	tst_syscall(__NR_readahead, 0, 0, 0);

	pagesize = getpagesize();

	setup_readahead_length();
	tst_res(TINFO, "readahead length: %d", readahead_length);

	ovl_mounted = TST_MOUNT_OVERLAY();
}

static void cleanup(void)
{
	if (ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);

	if (orig_bdi_limit)
		SAFE_FILE_PRINTF(sys_bdi_ra_path, "%d", orig_bdi_limit);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = mntpoint,
	.setup = setup,
	.cleanup = cleanup,
	.options = (struct tst_option[]) {
		{"s:", &opt_fsizestr, "Testfile size (default 64MB)"},
		{}
	},
	.test = test_readahead,
	.tcnt = ARRAY_SIZE(tcases),
	.max_runtime = 30,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "b833a3660394"},
		{"linux-git", "5b910bd615ba"},
		{}
	}
};
