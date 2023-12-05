// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023  Red Hat, Inc.
 */
/*\
 * [Description]
 *
 * Kernel Samepage Merging (KSM) for smart scan feature
 *
 * Test allocates a page and fills it with 'a' characters. It captures the
 * pages_skipped counter, waits for a few  iterations and captures the
 * pages_skipped counter again. The expectation  is that over 50% of the page
 * scans are skipped. (There is only one page that has KSM enabled and it gets
 * scanned during each iteration and it cannot be de-duplicated.)
 *
 * Smart scan feature was added in kernel v6.7.
 *
 * [Prerequisites]
 *
 * ksm and ksmtuned daemons need to be disabled. Otherwise, it could
 * distrub the testing as they also change some ksm tunables depends
 * on current workloads.
 */

#include <sys/wait.h>
#include "mem.h"

/* This test allocates one page, fills the page with a's, captures the
 * full_scan and pages_skipped counters. Then it makes sure at least 3
 * full scans have been performed and measures the above counters again.
 * The expectation is that at least 50% of the pages are skipped.
 *
 * To wait for at least 3 scans it uses the wait_ksmd_full_scan() function. In
 * reality, it will be a lot more scans as the wait_ksmd_full_scan() function
 * sleeps for one second.
 */
static void verify_ksm(void)
{
	int full_scans_begin;
	int full_scans_end;
	int pages_skipped_begin;
	int pages_skipped_end;
	int diff_pages;
	int diff_scans;
	unsigned long page_size;
	char *memory;

	/* Apply for the space for memory. */
	page_size = sysconf(_SC_PAGE_SIZE);
	memory = SAFE_MALLOC(page_size);
	memory = SAFE_MMAP(NULL, page_size, PROT_READ|PROT_WRITE,
					   MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#ifdef HAVE_DECL_MADV_MERGEABLE
	if (madvise(memory, page_size, MADV_MERGEABLE) == -1)
		tst_brk(TBROK|TERRNO, "madvise");
#endif
	memset(memory, 'a', page_size);

	tst_res(TINFO, "KSM merging");

	if (access(PATH_KSM "max_page_sharing", F_OK) == 0)
		SAFE_FILE_PRINTF(PATH_KSM "run", "2");

	/* Set defalut ksm scan values. */
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	SAFE_FILE_PRINTF(PATH_KSM "pages_to_scan", "%ld", 100l);
	SAFE_FILE_PRINTF(PATH_KSM "sleep_millisecs", "0");

	/* Measure pages skipped aka "smart scan". */
	SAFE_FILE_SCANF(PATH_KSM "full_scans", "%d", &full_scans_begin);
	SAFE_FILE_SCANF(PATH_KSM "pages_skipped", "%d", &pages_skipped_begin);
	wait_ksmd_full_scan();

	tst_res(TINFO, "stop KSM");
	SAFE_FILE_PRINTF(PATH_KSM "run", "0");

	SAFE_FILE_SCANF(PATH_KSM "full_scans", "%d", &full_scans_end);
	SAFE_FILE_SCANF(PATH_KSM "pages_skipped", "%d", &pages_skipped_end);
	diff_pages = pages_skipped_end - pages_skipped_begin;
	diff_scans = full_scans_end - full_scans_begin;

	if (diff_pages < diff_scans * 50 / 100) {
		tst_res(TINFO, "number of pages %d", diff_pages);
		tst_res(TINFO, "number of scans %d", diff_scans);
		tst_res(TFAIL, "not enough pages have been skipped by smart_scan");
	} else {
		tst_res(TPASS, "smart_scan skipped more than 50%% of the pages");
	}

#ifdef HAVE_DECL_MADV_MERGEABLE
	if (madvise(memory, page_size, MADV_UNMERGEABLE) == -1)
		tst_brk(TBROK|TERRNO, "madvise");
#endif
}

static struct tst_test test = {
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{}
	},
	.save_restore = (const struct tst_path_val[]) {
		{PATH_KSM "pages_skipped", NULL, TST_SR_TCONF},
		{PATH_KSM "run", NULL, TST_SR_TCONF},
		{PATH_KSM "sleep_millisecs", NULL, TST_SR_TCONF},
		{PATH_KSM "smart_scan", "1",
			TST_SR_SKIP_MISSING | TST_SR_TCONF},
		{}
	},
	.needs_kconfigs = (const char *const[]){
		"CONFIG_KSM=y",
		NULL
	},
	.test_all = verify_ksm,
};
