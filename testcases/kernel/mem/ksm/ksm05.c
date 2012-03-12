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
#include "usctest.h"
#include "mem.h"

char *TCID = "ksm05";
int TST_TOTAL = 1;

#ifdef HAVE_MADV_MERGEABLE

static int ksm_run_orig;

static void sighandler(int sig);
static void write_ksm_run(int val);

int main(int argc, char *argv[])
{
	int lc, status;
	long ps;
	char *msg;
	pid_t pid;
	void *ptr;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);

	ps = sysconf(_SC_PAGESIZE);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK|TERRNO, cleanup, "fork");
		case 0:
			if (posix_memalign(&ptr, ps, ps) < 0)
				tst_brkm(TBROK|TERRNO, cleanup,
						"posix_memalign");
			if (madvise(ptr, ps, MADV_MERGEABLE) < 0)
				tst_brkm(TBROK|TERRNO, cleanup, "madvise");
			*(char *)NULL = 0; /* SIGSEGV occurs as expected. */
			exit(0);
		default:
			break;
		}
		if (waitpid(pid, &status, WUNTRACED|WCONTINUED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
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

static void write_ksm_run(int val)
{
	int fd;
	char buf[BUFSIZ];

	sprintf(buf, "%d", val);
	fd = open(PATH_KSM "run", O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");
	if (write(fd, buf, 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write");
	close(fd);
}

void setup(void)
{
	int fd;
	char buf[BUFSIZ];

	tst_require_root(NULL);

	if (tst_kvercmp(2, 6, 32) < 0)
		tst_brkm(TCONF, NULL, "2.6.32 or greater kernel required.");

	tst_sig(FORK, sighandler, cleanup);
	TEST_PAUSE;

	/* save original /sys/kernel/mm/ksm/run value */
	fd = open(PATH_KSM "run", O_RDONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");
	if (read(fd, buf, 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "read");
	close(fd);
	ksm_run_orig = atoi(buf);

	/* echo 1 > /sys/kernel/mm/ksm/run */
	if (ksm_run_orig != 1)
		write_ksm_run(1);
}

void cleanup(void)
{
	/* restore /sys/kernel/mm/ksm/run value */
	if (ksm_run_orig != 1)
		write_ksm_run(ksm_run_orig);

	TEST_CLEANUP;
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "no MADV_MERGEABLE found.");
}
#endif
