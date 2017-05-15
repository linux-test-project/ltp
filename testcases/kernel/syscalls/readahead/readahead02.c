/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*
 * functional test for readahead() syscall
 *
 * This test is measuring effects of readahead syscall.
 * It mmaps/reads a test file with and without prior call to readahead.
 *
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "test.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"

char *TCID = "readahead02";
int TST_TOTAL = 1;

#if defined(__NR_readahead)
static const char testfile[] = "testfile";
static const char drop_caches_fname[] = "/proc/sys/vm/drop_caches";
static const char meminfo_fname[] = "/proc/meminfo";
static size_t testfile_size = 64 * 1024 * 1024;
static int opt_fsize;
static char *opt_fsizestr;
static int pagesize;

#define MIN_SANE_READAHEAD (4 * 1024)

option_t options[] = {
	{"s:", &opt_fsize, &opt_fsizestr},
	{NULL, NULL, NULL}
};

static void setup(void);
static void cleanup(void);

static void help(void)
{
	printf("  -s x    testfile size (default 64MB)\n");
}

static int check_ret(long expected_ret)
{
	if (expected_ret == TEST_RETURN) {
		tst_resm(TPASS, "expected ret success - "
			 "returned value = %ld", TEST_RETURN);
		return 0;
	}
	tst_resm(TFAIL | TTERRNO, "unexpected failure - "
		 "returned value = %ld, expected: %ld",
		 TEST_RETURN, expected_ret);
	return 1;
}

static int has_file(const char *fname, int required)
{
	int ret;
	struct stat buf;
	ret = stat(fname, &buf);
	if (ret == -1) {
		if (errno == ENOENT)
			if (required)
				tst_brkm(TCONF, cleanup, "%s not available",
					 fname);
			else
				return 0;
		else
			tst_brkm(TBROK | TERRNO, cleanup, "stat %s", fname);
	}
	return 1;
}

static void drop_caches(void)
{
	int ret;
	FILE *f;

	f = fopen(drop_caches_fname, "w");
	if (f) {
		ret = fprintf(f, "1");
		fclose(f);
		if (ret < 1)
			tst_brkm(TBROK, cleanup, "Failed to drop caches");
	} else {
		tst_brkm(TBROK, cleanup, "Failed to open drop_caches");
	}
}

static unsigned long parse_entry(const char *fname, const char *entry)
{
	FILE *f;
	long value = -1;
	int ret;
	char *line = NULL;
	size_t linelen;

	f = fopen(fname, "r");
	if (f) {
		do {
			ret = getline(&line, &linelen, f);
			if (sscanf(line, entry, &value) == 1)
				break;
		} while (ret != -1);
		fclose(f);
	}
	return value;
}

static unsigned long get_bytes_read(void)
{
	char fname[128];
	char entry[] = "read_bytes: %lu";
	sprintf(fname, "/proc/%u/io", getpid());
	return parse_entry(fname, entry);
}

static unsigned long get_cached_size(void)
{
	char entry[] = "Cached: %lu";
	return parse_entry(meminfo_fname, entry);
}

static void create_testfile(void)
{
	FILE *f;
	char *tmp;
	size_t i;

	tst_resm(TINFO, "creating test file of size: %ld", testfile_size);
	tmp = SAFE_MALLOC(cleanup, pagesize);

	/* round to page size */
	testfile_size = testfile_size & ~((long)pagesize - 1);

	f = fopen(testfile, "w");
	if (!f) {
		free(tmp);
		tst_brkm(TBROK | TERRNO, cleanup, "Failed to create %s",
			 testfile);
	}

	for (i = 0; i < testfile_size; i += pagesize)
		if (fwrite(tmp, pagesize, 1, f) < 1) {
			free(tmp);
			tst_brkm(TBROK, cleanup, "Failed to create %s",
				 testfile);
		}
	fflush(f);
	fsync(fileno(f));
	fclose(f);
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
static void read_testfile(int do_readahead, const char *fname, size_t fsize,
			  unsigned long *read_bytes, long *usec,
			  unsigned long *cached)
{
	int fd;
	size_t i = 0;
	long read_bytes_start;
	unsigned char *p, tmp;
	unsigned long time_start_usec, time_end_usec;
	unsigned long cached_start, max_ra_estimate = 0;
	off_t offset = 0;
	struct timeval now;

	fd = open(fname, O_RDONLY);
	if (fd < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "Failed to open %s", fname);

	if (do_readahead) {
		cached_start = get_cached_size();
		do {
			TEST(readahead(fd, offset, fsize - offset));
			if (TEST_RETURN != 0) {
				check_ret(0);
				break;
			}

			/* estimate max readahead size based on first call */
			if (!max_ra_estimate) {
				*cached = get_cached_size();
				if (*cached > cached_start) {
					max_ra_estimate = (1024 *
						(*cached - cached_start));
					tst_resm(TINFO, "max ra estimate: %lu",
						max_ra_estimate);
				}
				max_ra_estimate = MAX(max_ra_estimate,
					MIN_SANE_READAHEAD);
			}

			i++;
			offset += max_ra_estimate;
		} while ((size_t)offset < fsize);
		tst_resm(TINFO, "readahead calls made: %zu", i);
		*cached = get_cached_size();

		/* offset of file shouldn't change after readahead */
		offset = lseek(fd, 0, SEEK_CUR);
		if (offset == (off_t) - 1)
			tst_brkm(TBROK | TERRNO, cleanup, "lseek failed");
		if (offset == 0)
			tst_resm(TPASS, "offset is still at 0 as expected");
		else
			tst_resm(TFAIL, "offset has changed to: %lu", offset);
	}

	if (gettimeofday(&now, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "gettimeofday failed");
	time_start_usec = now.tv_sec * 1000000 + now.tv_usec;
	read_bytes_start = get_bytes_read();

	p = mmap(NULL, fsize, PROT_READ, MAP_SHARED | MAP_POPULATE, fd, 0);
	if (p == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap failed");

	/* for old kernels, where MAP_POPULATE doesn't work, touch each page */
	tmp = 0;
	for (i = 0; i < fsize; i += pagesize)
		tmp = tmp ^ p[i];
	/* prevent gcc from optimizing out loop above */
	if (tmp != 0)
		tst_brkm(TBROK, NULL, "This line should not be reached");

	if (!do_readahead)
		*cached = get_cached_size();

	if (munmap(p, fsize) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "munmap failed");

	*read_bytes = get_bytes_read() - read_bytes_start;
	if (gettimeofday(&now, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "gettimeofday failed");
	time_end_usec = now.tv_sec * 1000000 + now.tv_usec;
	*usec = time_end_usec - time_start_usec;

	if (close(fd) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "close failed");
}

static void test_readahead(void)
{
	unsigned long read_bytes, read_bytes_ra;
	long usec, usec_ra;
	unsigned long cached_max, cached_low, cached, cached_ra;
	char proc_io_fname[128];
	sprintf(proc_io_fname, "/proc/%u/io", getpid());

	/* find out how much can cache hold if we read whole file */
	read_testfile(0, testfile, testfile_size, &read_bytes, &usec, &cached);
	cached_max = get_cached_size();
	sync();
	drop_caches();
	cached_low = get_cached_size();
	cached_max = cached_max - cached_low;

	tst_resm(TINFO, "read_testfile(0)");
	read_testfile(0, testfile, testfile_size, &read_bytes, &usec, &cached);
	if (cached > cached_low)
		cached = cached - cached_low;
	else
		cached = 0;

	sync();
	drop_caches();
	cached_low = get_cached_size();
	tst_resm(TINFO, "read_testfile(1)");
	read_testfile(1, testfile, testfile_size, &read_bytes_ra,
		      &usec_ra, &cached_ra);
	if (cached_ra > cached_low)
		cached_ra = cached_ra - cached_low;
	else
		cached_ra = 0;

	tst_resm(TINFO, "read_testfile(0) took: %ld usec", usec);
	tst_resm(TINFO, "read_testfile(1) took: %ld usec", usec_ra);
	if (has_file(proc_io_fname, 0)) {
		tst_resm(TINFO, "read_testfile(0) read: %ld bytes", read_bytes);
		tst_resm(TINFO, "read_testfile(1) read: %ld bytes",
			 read_bytes_ra);
		/* actual number of read bytes depends on total RAM */
		if (read_bytes_ra < read_bytes)
			tst_resm(TPASS, "readahead saved some I/O");
		else
			tst_resm(TFAIL, "readahead failed to save any I/O");
	} else {
		tst_resm(TCONF, "Your system doesn't have /proc/pid/io,"
			 " unable to determine read bytes during test");
	}

	tst_resm(TINFO, "cache can hold at least: %ld kB", cached_max);
	tst_resm(TINFO, "read_testfile(0) used cache: %ld kB", cached);
	tst_resm(TINFO, "read_testfile(1) used cache: %ld kB", cached_ra);

	if (cached_max * 1024 >= testfile_size) {
		/*
		 * if cache can hold ~testfile_size then cache increase
		 * for readahead should be at least testfile_size/2
		 */
		if (cached_ra * 1024 > testfile_size / 2)
			tst_resm(TPASS, "using cache as expected");
		else
			tst_resm(TWARN, "using less cache than expected");
	} else {
		tst_resm(TCONF, "Page cache on your system is too small "
			 "to hold whole testfile.");
	}
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, help);

	if (opt_fsize)
		testfile_size = atoi(opt_fsizestr);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		test_readahead();
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	tst_tmpdir();
	TEST_PAUSE;

	has_file(drop_caches_fname, 1);
	has_file(meminfo_fname, 1);

	/* check if readahead is supported */
	ltp_syscall(__NR_readahead, 0, 0, 0);

	pagesize = getpagesize();
	create_testfile();
}

static void cleanup(void)
{
	unlink(testfile);
	tst_rmdir();
}

#else /* __NR_readahead */
int main(void)
{
	tst_brkm(TCONF, NULL, "System doesn't support __NR_readahead");
}
#endif
