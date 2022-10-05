// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) National ICT Australia, 2006
 * Author: carl.vanschaik at nicta.com.au
 */

/*\
 * [Description]
 *
 * If the kernel fails to correctly flush the TLB entry, the second mmap
 * will not show the correct data.
 *
 * [Algorithm]
 *  - create two files, write known data to the files
 *  - mmap the files, verify data
 *  - unmap files
 *  - remmap files, swap virtual addresses
 *  - check wheather if the memory content is correct
 */

#include <string.h>
#include "tst_test.h"

#define LEN 64

static int f1 = -1, f2 = -1;
static char *mm1 = NULL, *mm2 = NULL;

static const char tmp1[] = "testfile1";
static const char tmp2[] = "testfile2";

static const char str1[] = "testing 123";
static const char str2[] = "my test mem";

static void run(void)
{

	char *save_mm1, *save_mm2;

	mm1 = SAFE_MMAP(0, LEN, PROT_READ, MAP_PRIVATE, f1, 0);
	mm2 = SAFE_MMAP(0, LEN, PROT_READ, MAP_PRIVATE, f2, 0);

	save_mm1 = mm1;
	save_mm2 = mm2;

	if (strncmp(str1, mm1, strlen(str1)))
		tst_brk(TFAIL, "failed on compare %s", tmp1);

	if (strncmp(str2, mm2, strlen(str2)))
		tst_brk(TFAIL, "failed on compare %s", tmp2);

	SAFE_MUNMAP(mm1, LEN);
	SAFE_MUNMAP(mm2, LEN);

	mm1 = SAFE_MMAP(save_mm2, LEN, PROT_READ, MAP_PRIVATE, f1, 0);
	mm2 = SAFE_MMAP(save_mm1, LEN, PROT_READ, MAP_PRIVATE, f2, 0);

	if (mm1 != save_mm2 || mm2 != save_mm1)
		tst_res(TINFO, "mmap not using same address");

	if (strncmp(str1, mm1, strlen(str1)))
		tst_brk(TFAIL, "failed on compare %s", tmp1);

	if (strncmp(str2, mm2, strlen(str2)))
		tst_brk(TFAIL, "failed on compare %s", tmp2);

	tst_res(TPASS, "memory test succeeded");
}

static void setup(void)
{
	f1 = SAFE_OPEN(tmp1, O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	f2 = SAFE_OPEN(tmp2, O_RDWR | O_CREAT, S_IREAD | S_IWRITE);

	SAFE_WRITE(SAFE_WRITE_ALL, f1, str1, strlen(str1));
	SAFE_WRITE(SAFE_WRITE_ALL, f2, str2, strlen(str2));
}

static void cleanup(void)
{
	if (mm1)
		SAFE_MUNMAP(mm1, LEN);

	if (mm2)
		SAFE_MUNMAP(mm2, LEN);

	if (f1 != -1)
		SAFE_CLOSE(f1);
	if (f2 != -1)
		SAFE_CLOSE(f2);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};

