// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023, IBM Corporation.
 * Author: Tarun Sahu
 */

/*\
 * Before kernel version 5.10-rc7, there was a bug that resulted in a "Bad Page
 * State" error when freeing gigantic hugepages. This happened because the
 * struct page entry compound_nr, which overlapped with page->mapping in the
 * first tail page, was not cleared, causing the error. To ensure that this
 * issue does not reoccur as struct page keeps changing and some fields are
 * managed by folio, this test checks that freeing gigantic hugepages does not
 * produce the above-mentioned error.
 */

#define _GNU_SOURCE
#include <dirent.h>

#include <stdio.h>

#include "hugetlb.h"

#define PATH_HUGEPAGE "/sys/kernel/mm/hugepages"
#define GIGANTIC_MIN_ORDER 10

static int org_g_hpages;
static char g_hpage_path[4096];

static void run_test(void)
{
	if (FILE_PRINTF(g_hpage_path, "%d", 1))
		tst_brk(TCONF, "Can't update the gigantic hugepages.");
	SAFE_FILE_PRINTF(g_hpage_path, "%d", 0);

	if (tst_taint_check())
		tst_res(TFAIL, "Freeing Gigantic pages resulted in Bad Page State bug.");
	else
		tst_res(TPASS, "Successfully freed the gigantic hugepages");
}

static void setup(void)
{
	DIR *dir;
	struct dirent *ent;
	unsigned long hpage_size;

	if (access(PATH_HUGEPAGE, F_OK))
		tst_brk(TCONF, "hugetlbfs is not supported");

	dir = SAFE_OPENDIR(PATH_HUGEPAGE);
	while ((ent = SAFE_READDIR(dir))) {
		if ((sscanf(ent->d_name, "hugepages-%lukB", &hpage_size) == 1) &&
			is_hugetlb_gigantic(hpage_size * 1024)) {
			sprintf(g_hpage_path, "%s/%s/%s", PATH_HUGEPAGE,
					ent->d_name, "nr_hugepages");
			break;
		}
	}
	if (!g_hpage_path[0])
		tst_brk(TCONF, "Gigantic hugepages not supported");

	SAFE_CLOSEDIR(dir);

	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
	SAFE_FILE_PRINTF("/proc/sys/vm/compact_memory", "1");

	if (tst_available_mem() < (long long)hpage_size) {
		g_hpage_path[0] = '\0';
		tst_brk(TCONF, "No enough memory for gigantic hugepage reservation");
	}

	SAFE_FILE_LINES_SCANF(g_hpage_path, "%d", &org_g_hpages);
}

static void cleanup(void)
{
	if (g_hpage_path[0])
		SAFE_FILE_PRINTF(g_hpage_path, "%d", org_g_hpages);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
	    {"linux-git", "ba9c1201beaa"},
	    {"linux-git", "a01f43901cfb"},
	    {}
	},
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.taint_check = TST_TAINT_B,
};
