/*
 *  Copyright (c) International Business Machines  Corp., 2004
 *  Copyright (c) Linux Test Project, 2013-2014
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * This is a test for the madvise(2) system call. It is intended
 * to provide a complete exposure of the system call. It tests
 * madvise(2) for all error conditions to occur correctly.
 *
 * (A) Test Case for EINVAL
 *  1. start is not page-aligned
 *  2. advice is not a valid value
 *  3. application is attempting to release
 *     locked or shared pages (with MADV_DONTNEED)
 *  4. MADV_MERGEABLE or MADV_UNMERGEABLE was specified in advice,
 *     but the kernel was not configured with CONFIG_KSM.
 *
 * (B) Test Case for ENOMEM
 *  5. addresses in the specified range are not currently mapped
 *     or are outside the address space of the process
 *  b. Not enough memory - paging in failed
 *
 * (C) Test Case for EBADF
 *  6. the map exists,
 *     but the area maps something that isn't a file.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

#define TEST_FILE "testfile"
#define STR "abcdefghijklmnopqrstuvwxyz12345\n"

#define KSM_SYS_DIR	"/sys/kernel/mm/ksm"

static void setup(void);
static void cleanup(void);
static void check_and_print(int expected_errno);

static void test_addr_einval(void);
static void test_advice_einval(void);
#if !defined(UCLINUX)
static void test_lock_einval(void);
#endif /* if !defined(UCLINUX) */
#if defined(MADV_MERGEABLE)
static void test_mergeable_einval(void);
#endif
#if defined(MADV_UNMERGEABLE)
static void test_unmergeable_einval(void);
#endif
static void test_enomem(void);
static void test_ebadf(void);

static void (*test_func[])(void) = {
	test_addr_einval,
	test_advice_einval,
#if !defined(UCLINUX)
	test_lock_einval,
#endif /* if !defined(UCLINUX) */
#if defined(MADV_MERGEABLE)
	test_mergeable_einval,
#endif
#if defined(MADV_UNMERGEABLE)
	test_unmergeable_einval,
#endif
	test_enomem,
	test_ebadf,
};

char *TCID = "madvise02";
int TST_TOTAL = ARRAY_SIZE(test_func);

static int fd;
static struct stat st;
static int pagesize;

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*test_func[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, TEST_FILE, O_RDWR | O_CREAT, 0664);

	pagesize = getpagesize();

	/* Writing 16 pages of random data into this file */
	for (i = 0; i < (pagesize / 2); i++)
		SAFE_WRITE(cleanup, 1, fd, STR, sizeof(STR) - 1);

	SAFE_FSTAT(cleanup, fd, &st);
}

static void cleanup(void)
{
	if (fd && close(fd) < 0)
		tst_resm(TWARN | TERRNO, "close failed");

	tst_rmdir();
}

static void check_and_print(int expected_errno)
{
	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == expected_errno)
			tst_resm(TPASS | TTERRNO, "failed as expected");
		else
			tst_resm(TFAIL | TTERRNO,
				 "failed unexpectedly; expected - %d : %s",
				 expected_errno, strerror(expected_errno));
	} else {
		tst_resm(TFAIL, "madvise succeeded unexpectedly");
	}
}

static void test_addr_einval(void)
{
	char *file;

	file = SAFE_MMAP(cleanup, 0, st.st_size, PROT_READ,
					 MAP_SHARED, fd, 0);

	TEST(madvise(file + 100, st.st_size, MADV_NORMAL));
	check_and_print(EINVAL);

	SAFE_MUNMAP(cleanup, file, st.st_size);
}

static void test_advice_einval(void)
{
	char *file;

	file = SAFE_MMAP(cleanup, 0, st.st_size, PROT_READ,
					 MAP_SHARED, fd, 0);

	TEST(madvise(file, st.st_size, 1212));
	check_and_print(EINVAL);

	SAFE_MUNMAP(cleanup, file, st.st_size);
}

#if !defined(UCLINUX)
static void test_lock_einval(void)
{
	char *file;

	file = SAFE_MMAP(cleanup, 0, st.st_size, PROT_READ,
					 MAP_SHARED, fd, 0);

	if (mlock(file, st.st_size) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "mlock failed");

	TEST(madvise(file, st.st_size, MADV_DONTNEED));
	check_and_print(EINVAL);

	SAFE_MUNMAP(cleanup, file, st.st_size);
}
#endif /* if !defined(UCLINUX) */

#if defined(MADV_MERGEABLE)
static void test_mergeable_einval(void)
{
	char *file;

	if (access(KSM_SYS_DIR, F_OK) >= 0) {
		tst_resm(TCONF, "kernel configured with CONFIG_KSM, "
				 "skip EINVAL test for MADV_MERGEABLE.");
		return;
	}

	file = SAFE_MMAP(cleanup, 0, st.st_size, PROT_READ,
					 MAP_SHARED, fd, 0);

	TEST(madvise(file, st.st_size, MADV_MERGEABLE));
	check_and_print(EINVAL);

	SAFE_MUNMAP(cleanup, file, st.st_size);
}
#endif

#if defined(MADV_UNMERGEABLE)
static void test_unmergeable_einval(void)
{
	char *file;

	if (access(KSM_SYS_DIR, F_OK) >= 0) {
		tst_resm(TCONF, "kernel configured with CONFIG_KSM, "
				 "skip EINVAL test for MADV_UNMERGEABLE.");
		return;
	}

	file = SAFE_MMAP(cleanup, 0, st.st_size, PROT_READ,
					 MAP_SHARED, fd, 0);

	TEST(madvise(file, st.st_size, MADV_UNMERGEABLE));
	check_and_print(EINVAL);

	SAFE_MUNMAP(cleanup, file, st.st_size);
}
#endif

static void test_enomem(void)
{
	char *low;
	char *high;
	unsigned long len;

	low = SAFE_MMAP(cleanup, 0, st.st_size / 2, PROT_READ,
					MAP_SHARED, fd, 0);

	high = SAFE_MMAP(cleanup, 0, st.st_size / 2, PROT_READ,
					 MAP_SHARED, fd, st.st_size / 2);

	/* Swap if necessary to make low < high */
	if (low > high) {
		char *tmp;
		tmp = high;
		high = low;
		low = tmp;
	}

	len = (high - low) + pagesize;

	SAFE_MUNMAP(cleanup, high, st.st_size / 2);

	TEST(madvise(low, len, MADV_NORMAL));
	check_and_print(ENOMEM);

	SAFE_MUNMAP(cleanup, low, st.st_size / 2);
}

static void test_ebadf(void)
{
	char *ptr_memory_allocated = NULL;
	char *tmp_memory_allocated = NULL;

	/* Create one memory segment using malloc */
	ptr_memory_allocated = malloc(5 * pagesize);
	/*
	 * Take temporary pointer for later use, freeing up the
	 * original one.
	 */
	tmp_memory_allocated = ptr_memory_allocated;
	tmp_memory_allocated =
		(char *)(((unsigned long)tmp_memory_allocated +
			pagesize - 1) & ~(pagesize - 1));

	TEST(madvise(tmp_memory_allocated, 5 * pagesize, MADV_WILLNEED));
	if (tst_kvercmp(3, 9, 0) < 0) {
		check_and_print(EBADF);
	/* in kernel commit 1998cc0, madvise(MADV_WILLNEED) to anon
	 * mem doesn't return -EBADF now, as now we support swap
	 * prefretch.
	 */
	} else {
		tst_resm(TPASS, "madvise succeeded as expected, see "
				"kernel commit 1998cc0 for details.");
	}

	free(ptr_memory_allocated);
}
