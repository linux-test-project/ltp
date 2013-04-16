/*
 * Copyright (C) 2013 Linux Test Project
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
 * The case is designed to test the functionality of transparent
 * hugepage - THP
 *
 * when one process allocate hugepage aligned anonymous pages,
 * kernel thread 'khugepaged' controlled by sysfs knobs
 * /sys/kernel/mm/transparent_hugepage/ will scan them, and make
 * them as transparent hugepage if they are suited, you can find out
 * how many transparent hugepages are there in one process from
 * /proc/<pid>/smaps, among the file contents, 'AnonHugePages' entry
 * stand for transparent hugepage.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"
#include "mem.h"

char *TCID = "thp04";
int TST_TOTAL = 1;

option_t thp_options[] = {
	{"n:", &opt_nr_children, &opt_nr_children_str},
	{"N:", &opt_nr_thps, &opt_nr_thps_str},
	{NULL, NULL, NULL}
};

static int pre_thp_scan_sleep_millisecs;
static int pre_thp_alloc_sleep_millisecs;
static char pre_thp_enabled[BUFSIZ];

int main(int argc, char *argv[])
{
	int lc;
	char *msg;
	int nr_children = 2, nr_thps = 64;

	msg = parse_opts(argc, argv, thp_options, thp_usage);
	if (msg != NULL)
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	check_thp_options(&nr_children, &nr_thps);

	setup();

	tst_resm(TINFO, "Start to test transparent hugepage...");
	tst_resm(TINFO, "There are %d children allocating %d "
			"transparent hugepages", nr_children, nr_thps);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		test_transparent_hugepage(nr_children, nr_thps, 1, 0);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root(NULL);

	if (access(PATH_THP, F_OK) == -1)
		tst_brkm(TCONF, NULL, "THP is not enabled");

	SAFE_FILE_SCANF(NULL, PATH_KHPD "scan_sleep_millisecs",
			"%d", &pre_thp_scan_sleep_millisecs);
	/* set 0 to khugepaged/scan_sleep_millisecs to run khugepaged 100% */
	SAFE_FILE_PRINTF(NULL, PATH_KHPD "scan_sleep_millisecs", "0");

	SAFE_FILE_SCANF(NULL, PATH_KHPD "alloc_sleep_millisecs",
			"%d", &pre_thp_alloc_sleep_millisecs);
	/*
	 * set 0 to khugepaged/alloc_sleep_millisecs to make sure khugepaged
	 * don't stop if there's a hugepage allcation failure.
	 */
	SAFE_FILE_PRINTF(NULL, PATH_KHPD "alloc_sleep_millisecs", "0");

	SAFE_FILE_SCANF(NULL, PATH_THP "enabled", "%[^\n]", pre_thp_enabled);
	/* open khugepaged as 'always' mode */
	SAFE_FILE_PRINTF(NULL, PATH_THP "enabled", "always");

	tst_sig(FORK, DEF_HANDLER, NULL);
	TEST_PAUSE;
}

void cleanup(void)
{
	SAFE_FILE_PRINTF(NULL, PATH_KHPD "scan_sleep_millisecs",
			 "%d", pre_thp_scan_sleep_millisecs);

	SAFE_FILE_PRINTF(NULL, PATH_KHPD "alloc_sleep_millisecs",
			 "%d", pre_thp_alloc_sleep_millisecs);

	/*
	 * The value of transparent_hugepage/enabled is speical,
	 * we need to recover the previous value one by one for
	 * the three mode: always, madvise, never.
	 */
	if (strcmp(pre_thp_enabled, "[always] madvise never") == 0)
		SAFE_FILE_PRINTF(NULL, PATH_THP "enabled", "always");
	else if (strcmp(pre_thp_enabled, "always [madvise] never") == 0)
		SAFE_FILE_PRINTF(NULL, PATH_THP "enabled", "madvise");
	else
		SAFE_FILE_PRINTF(NULL, PATH_THP "enabled", "never");

	TEST_CLEANUP;
}
