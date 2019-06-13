// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-2.0-or-later

/*
 *  Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 *  Email: code@zilogic.com
 */

/*
 * Test: Validating memfd_create() with MFD_HUGETLB flag.
 *
 * Test case 1: --WRITE CALL IN HUGEPAGES TEST--
 *              Huge pages are write protected. Any writes to
 *              the file should return EINVAL error.
 *
 * Test case 2: --PAGE SIZE OF CREATED FILE TEST--
 *              Default huge page sized pages are created with
 *              MFD_HUGETLB flag. Any attempt to unmap memory-mapped
 *              huge pages with an unmapping length less than
 *              huge page size should return EINVAL error.
 *
 * Test case 3: --HUGEPAGE ALLOCATION LIMIT TEST--
 *              Number of huge pages currently available to use should be
 *              atmost total number of allowed huge pages. Memory-mapping
 *              more than allowed huge pages should return ENOMEM error.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "memfd_create_common.h"

#include <stdio.h>
#include <errno.h>

#define TOTAL_HP_PATH "/proc/sys/vm/nr_hugepages"
#define MEMINFO_PATH "/proc/meminfo"
#define FREE_HP "HugePages_Free:\t%ld"
#define DEFAULT_HPS "Hugepagesize:\t%ld kB"

static int hugepages_allocated;
static long og_total_pages;

static void *check_huge_mmapable(int fd, unsigned long size)
{
	void *mem;

	mem = SAFE_MMAP(NULL, size, PROT_WRITE, MAP_PRIVATE, fd, 0);

	memset((char *)mem, 0, 1);

	tst_res(TINFO,
		"mmap(%p, %lu, %d, %d, %d, %d) succeeded",
		NULL, size, PROT_WRITE, MAP_PRIVATE, fd, 0);

	return mem;
}

static void test_write_protect(int fd)
{
	ssize_t ret;
	char test_str[] = "LTP";

	ret = write(fd, test_str, strlen(test_str));
	if (ret < 0) {
		if (errno != EINVAL) {
			tst_res(TFAIL | TERRNO,
				"write(%d, \"%s\", %zu) didn't fail as expected\n",
				fd, test_str, strlen(test_str));
			return;
		}
	} else {
		tst_res(TFAIL,
			"write(%d, \"%s\", %zu) succeeded unexpectedly\n",
			fd, test_str, strlen(test_str));
		return;
	}

	tst_res(TPASS,
		"write(%d, \"%s\", %zu) failed as expected\n",
		fd, test_str, strlen(test_str));
}

static void test_def_pagesize(int fd)
{
	unsigned int i;
	int unmap_size;
	int ret;
	long hps;
	void *mem;

	SAFE_FILE_LINES_SCANF(MEMINFO_PATH, DEFAULT_HPS, &hps);
	hps = hps << 10;
	unmap_size = hps / 4;
	mem = check_huge_mmapable(fd, hps);

	for (i = unmap_size; i < hps; i += unmap_size) {
		ret = munmap(mem, i);
		if (ret == -1) {
			if (errno == EINVAL) {
				tst_res(TINFO,
					"munmap(%p, %dkB) failed as expected",
					mem, i/1024);
			} else {
				tst_res(TFAIL | TERRNO,
					"munmap(%p, %dkB) failed unexpectedly",
					mem, i/1024);
				return;
			}
		} else {
			tst_res(TFAIL,
				"munmap(%p, %dkB) suceeded unexpectedly\n",
				mem, i);
			return;
		}
	}

	SAFE_MUNMAP(mem, hps);

	tst_res(TPASS,
		"munmap() fails for page sizes less than %ldkB\n", hps/1024);
}

static void test_max_hugepages(int fd)
{
	int new_fd;
	long hps;
	long free_pages;
	void *mem;
	void *new_mem;

	SAFE_FILE_LINES_SCANF(MEMINFO_PATH, FREE_HP, &free_pages);
	SAFE_FILE_LINES_SCANF(MEMINFO_PATH, DEFAULT_HPS, &hps);
	hps = hps << 10;
	mem = check_huge_mmapable(fd, free_pages * hps);

	new_fd = sys_memfd_create("new_file", MFD_HUGETLB);
	if (new_fd < 0)
		tst_brk(TFAIL | TERRNO, "memfd_create() failed");
	tst_res(TINFO, "memfd_create() succeeded");

	new_mem = mmap(NULL, hps, 0, MAP_PRIVATE, new_fd, 0);
	if (new_mem == MAP_FAILED) {
		if (errno == ENOMEM)
			tst_res(TPASS,
				"mmap(%p, %lu, %d, %d, %d, %d) failed as expected",
				NULL, hps, 0, MAP_PRIVATE, new_fd, 0);
		else
			tst_res(TFAIL | TERRNO,
				"mmap(%p, %lu, %d, %d, %d, %d) failed unexpectedly",
				NULL, hps, 0, MAP_PRIVATE, new_fd, 0);
	} else {
		tst_res(TFAIL,
			"mmap(%p, %lu, %d, %d, %d, %d) succeeded",
			NULL, hps, 0, MAP_PRIVATE, new_fd, 0);
		SAFE_MUNMAP(new_mem, hps);
	}

	SAFE_CLOSE(new_fd);

	SAFE_MUNMAP(mem, free_pages * hps);
}

static const struct tcase {
	void (*func)(int fd);
	const char *desc;
} tcases[] = {
	{&test_write_protect,   "--TESTING WRITE CALL IN HUGEPAGES--"},
	{&test_def_pagesize,  "--TESTING PAGE SIZE OF CREATED FILE--"},
	{&test_max_hugepages, "--TESTING HUGEPAGE ALLOCATION LIMIT--"},
};

static void memfd_huge_controller(unsigned int n)
{
	int fd;
	const struct tcase *tc;

	tc = &tcases[n];

	tst_res(TINFO, "%s", tc->desc);

	fd = sys_memfd_create("test_file", MFD_HUGETLB);
	if (fd < 0)
		tst_brk(TFAIL | TERRNO, "memfd_create() failed");
	tst_res(TINFO, "memfd_create() succeeded");

	tc->func(fd);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	char buf[8];
	int fd;
	long free_pages;
	long total_pages;

	if (access(MEMINFO_PATH, F_OK) ||
	    access("/sys/kernel/mm/hugepages", F_OK) ||
	    access(TOTAL_HP_PATH, F_OK))
		tst_brk(TCONF, "Huge page is not supported");

	SAFE_FILE_LINES_SCANF(MEMINFO_PATH, FREE_HP, &free_pages);
	if (free_pages > 0)
		return;

	SAFE_FILE_LINES_SCANF(TOTAL_HP_PATH, "%ld", &og_total_pages);
	sprintf(buf, "%ld", og_total_pages + 1);

	fd = open(TOTAL_HP_PATH, O_RDWR | O_TRUNC);

	if (write(fd, buf, strlen(buf)) == -1)
		tst_brk(TCONF | TERRNO,
			"write() fail: Hugepage allocation failed");

	SAFE_CLOSE(fd);

	SAFE_FILE_LINES_SCANF(TOTAL_HP_PATH, "%ld", &total_pages);
	if (total_pages != (og_total_pages + 1))
		tst_brk(TCONF, "Hugepage allocation failed");

	hugepages_allocated = 1;
}

static void cleanup(void)
{
	char buf[8];
	int fd;
	long total_pages;

	if (hugepages_allocated == 0)
		return;

	sprintf(buf, "%ld", og_total_pages);

	fd = open(TOTAL_HP_PATH, O_RDWR | O_TRUNC);

	if (write(fd, buf, strlen(buf)) == -1)
		tst_brk(TCONF | TERRNO, "Clean-up failed: write() failed");

	SAFE_CLOSE(fd);

	SAFE_FILE_LINES_SCANF(TOTAL_HP_PATH, "%ld", &total_pages);
	if (og_total_pages != total_pages)
		tst_brk(TCONF, "Clean-up failed");
}

static struct tst_test test = {
	.setup = setup,
	.test = memfd_huge_controller,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.min_kver = "4.14",
	.cleanup = cleanup,
};
