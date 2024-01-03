// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) International Business Machines Corp., 2004
 *  Copyright (c) Linux Test Project, 2013-2016
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
 *  8|9. The MADV_FREE & MADV_WIPEONFORK operation can be applied
 *  	only to private anonymous pages.
 *
 * (B) Test Case for ENOMEM
 *  5|6. addresses in the specified range are not currently mapped
 *     or are outside the address space of the process
 *  b. Not enough memory - paging in failed
 *
 * (C) Test Case for EBADF
 *  7. the map exists,
 *     but the area maps something that isn't a file.
 */

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tst_test.h"
#include "lapi/mmap.h"

#define MAP_SIZE (4 * 1024)
#define TEST_FILE "testfile"
#define STR "abcdefghijklmnopqrstuvwxyz12345\n"
#define KSM_SYS_DIR	"/sys/kernel/mm/ksm"

static struct stat st;
static long pagesize;
static char *file1;
static char *file2;
static char *file3;
static char *shared_anon;
static char *ptr_addr;
static char *tmp_addr;
static char *nonalign;

#define TCASE(adv, nm, add, ee) {adv, nm, add, ee, 0}

static struct tcase {
	int advice;
	char *name;
	char **addr;
	int exp_errno;
	int skip;
} tcases[] = {
	TCASE(MADV_NORMAL,      "MADV_NORMAL",      &nonalign, EINVAL),
	TCASE(1212,             "MADV_NORMAL",      &file1,    EINVAL),
	TCASE(MADV_REMOVE,      "MADV_REMOVE",      &file1,    EINVAL),
	TCASE(MADV_DONTNEED,    "MADV_DONTNEED",    &file1,    EINVAL),
	TCASE(MADV_MERGEABLE,   "MADV_MERGEABLE",   &file1,    EINVAL),
	TCASE(MADV_UNMERGEABLE, "MADV_UNMERGEABLE", &file1,    EINVAL),
	TCASE(MADV_NORMAL,      "MADV_NORMAL",      &file2,    ENOMEM),
	TCASE(MADV_WILLNEED,    "MADV_WILLNEED",    &file2,    ENOMEM),
	TCASE(MADV_WILLNEED,    "MADV_WILLNEED",    &tmp_addr,  EBADF),
	TCASE(MADV_FREE,        "MADV_FREE",        &file1,    EINVAL),
	TCASE(MADV_WIPEONFORK,  "MADV_WIPEONFORK",  &file1,    EINVAL),
	TCASE(MADV_WIPEONFORK,  "MADV_WIPEONFORK shared_anon", &shared_anon, EINVAL),
	TCASE(MADV_WIPEONFORK,  "MADV_WIPEONFORK private file backed", &file3, EINVAL),
};

static void tcases_filter(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		struct tcase *tc = &tcases[i];

		switch (tc->advice) {
		case MADV_DONTNEED:
			if (mlock(file1, st.st_size) < 0)
				tst_brk(TBROK | TERRNO, "mlock failed");
		break;
		case MADV_MERGEABLE:
		case MADV_UNMERGEABLE:
			/* kernel configured with CONFIG_KSM,
			 * skip EINVAL test for MADV_MERGEABLE. */
			if (access(KSM_SYS_DIR, F_OK) == 0)
				tc->skip = 1;
		break;
		case MADV_WILLNEED:
			/* In kernel commit 1998cc0, madvise(MADV_WILLNEED) to
			 * anon mem doesn't return -EBADF now, as now we support
			 * swap prefretch. */
			if (tc->exp_errno == EBADF)
				tc->skip = 1;
		break;
		case MADV_FREE:
			if ((tst_kvercmp(4, 5, 0)) < 0)
				tc->skip = 1;
		break;
		case MADV_WIPEONFORK:
			if ((tst_kvercmp(4, 14, 0)) < 0)
				tc->skip = 1;
		break;
		default:
		break;
		}
	}
}

static void setup(void)
{
	int i, fd;

	fd = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT, 0664);

	pagesize = getpagesize();

	/* Writing 16 pages of random data into this file */
	for (i = 0; i < (pagesize / 2); i++)
		SAFE_WRITE(SAFE_WRITE_ALL, fd, STR, sizeof(STR) - 1);

	SAFE_FSTAT(fd, &st);

	file1 = SAFE_MMAP(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	file2 = SAFE_MMAP(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	file3 = SAFE_MMAP(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	shared_anon = SAFE_MMAP(0, MAP_SIZE, PROT_READ, MAP_SHARED |
			MAP_ANONYMOUS, -1, 0);

	nonalign = file1 + 100;

	ptr_addr = SAFE_MALLOC(st.st_size);
	tmp_addr = (void*)LTP_ALIGN((long)ptr_addr, pagesize);

	/* unmap as last step to avoid subsequent mmap(s) pick same address */
	SAFE_MUNMAP(file2 + st.st_size - pagesize, pagesize);
	SAFE_CLOSE(fd);

	tcases_filter();
}


static void advice_test(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	if (tc->skip == 1) {
		tst_res(TCONF, "%s is not supported", tc->name);
		return;
	}

	TEST(madvise(*(tc->addr), st.st_size, tc->advice));
	if (TST_RET == -1) {
		if (TST_ERR == tc->exp_errno) {
			tst_res(TPASS | TTERRNO, "%s failed as expected", tc->name);
		} else {
			tst_res(TFAIL | TTERRNO,
					"%s failed unexpectedly; expected - %d : %s",
					tc->name, tc->exp_errno,
					tst_strerrno(TFAIL | TTERRNO));
		}
	} else {
		tst_res(TFAIL, "madvise succeeded unexpectedly");
	}
}

static void cleanup(void)
{
	free(ptr_addr);
	SAFE_MUNMAP(file1, st.st_size);
	SAFE_MUNMAP(file2, st.st_size - pagesize);
	SAFE_MUNMAP(file3, st.st_size);
	SAFE_MUNMAP(shared_anon, MAP_SIZE);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = advice_test,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
};
