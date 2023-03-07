// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) Zilogic Systems Pvt. Ltd. <code@zilogic.com>, 2018
 * Copyright (c) Linux Test Project, 2019-2023
 */

/*\
 * [Description]
 *
 * Validating memfd_create() with MFD_HUGETLB and MFD_HUGE_x flags.
 *
 * Attempt to create files in the hugetlbfs filesystem using different huge page
 * sizes.
 *
 * [Algorithm]
 *
 * memfd_create() should return non-negative value (fd) if the system supports
 * that particular huge page size.
 * On success, fd is returned. On failure, -1 is returned with ENODEV error.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "memfd_create_common.h"

#include <errno.h>
#include <stdio.h>

static  struct test_flag {
	int flag;
	char *h_size;
	int exp_err;
} test_flags[] =  {
	{.flag = MFD_HUGE_64KB,         .h_size = "64kB"},
	{.flag = MFD_HUGE_512KB,       .h_size = "512kB"},
	{.flag = MFD_HUGE_2MB,        .h_size = "2048kB"},
	{.flag = MFD_HUGE_8MB,        .h_size = "8192kB"},
	{.flag = MFD_HUGE_16MB,      .h_size = "16384kB"},
	{.flag = MFD_HUGE_256MB,    .h_size = "262144kB"},
	{.flag = MFD_HUGE_1GB,     .h_size = "1048576kB"},
	{.flag = MFD_HUGE_2GB,     .h_size = "2097152kB"},
	{.flag = MFD_HUGE_16GB,   .h_size = "16777216kB"},
};

static void check_hugepage_support(struct test_flag *test_flags)
{
	char pattern[64];

	sprintf(pattern, PATH_HUGEPAGES);
	strcat(pattern, "hugepages-");
	strcat(pattern, test_flags->h_size);

	if (access(pattern, F_OK))
		test_flags->exp_err = ENODEV;
}

static void memfd_huge_x_controller(unsigned int n)
{
	int fd;
	struct test_flag tflag;

	tflag = test_flags[n];
	check_hugepage_support(&tflag);
	tst_res(TINFO,
		"Attempt to create file using %s huge page size",
		tflag.h_size);

	fd = sys_memfd_create("tfile", MFD_HUGETLB | tflag.flag);
	if (fd < 0) {
		if (errno == tflag.exp_err)
			tst_res(TPASS, "Test failed as expected");
		else
			tst_brk(TFAIL | TERRNO,
				"memfd_create() failed unexpectedly");
		return;
	}

	tst_res(TPASS,
		"memfd_create succeeded for %s page size",
		tflag.h_size);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	if (access(PATH_HUGEPAGES, F_OK))
		tst_brk(TCONF, "Huge page is not supported");
}

static struct tst_test test = {
	.setup = setup,
	.test = memfd_huge_x_controller,
	.tcnt = ARRAY_SIZE(test_flags),
	.min_kver = "4.14",
};
