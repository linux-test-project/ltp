/*
 * KSM - NULL pointer dereference in ksm_do_scan() (CVE-2011-2183)
 *
 * This is a testcase from upstream commit:
 * 2b472611a32a72f4a118c069c2d62a1a3f087afd.
 *
 * an exiting task can race against ksmd::scan_get_next_rmap_item
 * (http://lkml.org/lkml/2011/6/1/742) easily triggering a NULL pointer
 * dereference in ksmd.
 * ksm_scan.mm_slot == &ksm_mm_head with only one registered mm
 *
 * CPU 1 (__ksm_exit)          CPU 2 (scan_get_next_rmap_item)
 *                             list_empty() is false
 * lock                        slot == &ksm_mm_head
 * list_del(slot->mm_list)
 * (list now empty)
 * unlock
 *                              lock
 *                              slot = list_entry(slot->mm_list.next)
 *                              (list is empty, so slot is still ksm_mm_head)
 *                              unlock
 *                              slot->mm == NULL ... Oops
 *
 * Close this race by revalidating that the new slot is not simply the list
 * head again.
 *
 * Test Prerequisites:
 *
 * *) ksm and ksmtuned daemons need to be disabled. Otherwise, it could
 *    distrub the testing as they also change some ksm tunables depends
 *    on current workloads.
 *
 * Copyright (C) 2011  Red Hat, Inc.
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
#include "mem.h"

char *TCID = "ksm05";
int TST_TOTAL = 1;

#ifdef HAVE_MADV_MERGEABLE

static int ksm_run_orig;

static void sighandler(int sig);

int main(int argc, char *argv[])
{
	int lc, status, ret;
	long ps;
	pid_t pid;
	void *ptr;

	tst_parse_opts(argc, argv, NULL, NULL);

	ps = sysconf(_SC_PAGESIZE);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			ret = posix_memalign(&ptr, ps, ps);
			if (ret) {
				tst_brkm(TBROK, cleanup, "posix_memalign(): %s",
				         tst_strerrno(ret));
			}
			if (madvise(ptr, ps, MADV_MERGEABLE) < 0)
				tst_brkm(TBROK | TERRNO, cleanup, "madvise");
			*(char *)NULL = 0;	/* SIGSEGV occurs as expected. */
			exit(0);
		default:
			break;
		}
		if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_brkm(TBROK, cleanup,
				 "invalid signal received: %d", status);
	}

	tst_resm(TPASS, "still alive.");
	cleanup();
	tst_exit();
}

static void sighandler(int sig)
{
	_exit((sig == SIGSEGV) ? 0 : sig);
}

void setup(void)
{
	tst_require_root();

	if (tst_kvercmp(2, 6, 32) < 0)
		tst_brkm(TCONF, NULL, "2.6.32 or greater kernel required.");

	if (access(PATH_KSM, F_OK) == -1)
		tst_brkm(TCONF, NULL, "KSM configuration is not enabled");

	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;

	/* save original /sys/kernel/mm/ksm/run value */
	SAFE_FILE_SCANF(NULL, PATH_KSM "run", "%d", &ksm_run_orig);

	/* echo 1 > /sys/kernel/mm/ksm/run */
	SAFE_FILE_PRINTF(NULL, PATH_KSM "run", "1");
}

void cleanup(void)
{
	/* restore /sys/kernel/mm/ksm/run value */
	FILE_PRINTF(PATH_KSM "run", "%d", ksm_run_orig);
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "no MADV_MERGEABLE found.");
}
#endif
