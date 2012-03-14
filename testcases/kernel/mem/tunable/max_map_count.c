/*
 * The program is designed to test max_map_count tunable file
 *
 * The kernel Documentation say that:
 * /proc/sys/vm/max_map_count contains the maximum number of memory map
 * areas a process may have. Memory map areas are used as a side-effect
 * of calling malloc, directly by mmap and mprotect, and also when
 * loading shared libraries.
 *
 * Each process has his own maps file: /proc/[pid]/maps, and each line
 * indicates a map entry, so it can caculate the amount of maps by reading
 * the file lines' number to check the tunable performance.
 *
 * The program trys to invoke mmap() endless until triggering MAP_FAILED,
 * then read the process's maps file /proc/[pid]/maps, save the line number
 * to map_count variable, and compare it with /proc/sys/vm/max_map_count,
 * map_count should less than max_map_count.
 * Note: There are two special vmas VDSO and VSYSCALL, which are allocated
 * via install_special_mapping(), install_specail_mapping() allows the VMAs
 * to be allocated and inserted without checking the sysctl_map_map_count,
 * and each /proc/<pid>/maps has both at the end:
 * # cat /proc/self/maps
 * ...
 * ...
 * 7fff7b9ff000-7fff7ba00000 r-xp 00000000 00:00 0           [vdso]
 * ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0   [vsyscall]
 *
 * so during comparing with map_count and /proc/sys/vm/max_map_count,
 * we should except the two special vmas from map_count:
 * map_count -= 2;
 *
 * ********************************************************************
 * Copyright (C) 2012 Red Hat, Inc.
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
 *
 * ********************************************************************
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"
#include "mem.h"

#define MAX_MAP_COUNT_DEFAULT 64

char *TCID = "max_map_count";
int TST_TOTAL = 1;

static long old_max_map_count;

static long count_maps(pid_t pid);
static void max_map_count_test(void);

int main(int argc, char *argv[])
{
	char *msg;
	int lc;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR -%s ", msg);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;
		max_map_count_test();
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	if (access(PATH_SYSVM "max_map_count", F_OK) != 0)
		tst_brkm(TBROK | TERRNO, NULL,
			 "Can't support to test max_map_count");

	old_max_map_count = get_sys_tune("max_map_count");
}

void cleanup(void)
{
	set_sys_tune("max_map_count", old_max_map_count, 0);

	TEST_CLEANUP;
}

static long count_maps(pid_t pid)
{
	FILE *fp;
	size_t len;
	char *line = NULL;
	char buf[BUFSIZ];
	long map_count = 0;

	snprintf(buf, BUFSIZ, "/proc/%d/maps", pid);
	fp = fopen(buf, "r");
	if (fp == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "fopen %s", buf);
	while (getline(&line, &len, fp) != -1) {
		/* exclude vdso and vsyscall */
		if (sscanf(line, "%*p-%*p %*4s %*p %*2d:%*2d %*d %s", buf) ==
			    1 && ((strcmp(buf, "[vdso]") == 0) ||
				  (strcmp(buf, "[vsyscall]") == 0)))
			continue;
		map_count++;
	}
	fclose(fp);

	return map_count;
}

static void max_map_count_test(void)
{
	int i;
	int status;
	pid_t pid;
	long max_maps;
	long map_count;
	long max_iters;

	/*
	 * XXX Due to a possible kernel bug, oom-killer can be easily
	 * triggered when doing small piece mmaps in huge amount even if
	 * many swap is available and/or overcommit_memory set to 2.
	 * Also oom-killer would kill wrong victims in this situation,
	 * we only test with all free physical memory for now. After
	 * oom-killer becomes more stable (which is unlikely to be), we
	 * can consider to test all available physical mem + swap.
	 */
	max_iters = read_meminfo("MemFree:") / sysconf(_SC_PAGESIZE) * 1024;

	max_maps = MAX_MAP_COUNT_DEFAULT;
	for (i = 0; i < 5; i++) {
		if (max_maps > max_iters)
			max_maps = max_iters;
		set_sys_tune("max_map_count", max_maps, 1);

		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			while (mmap(NULL, 1, PROT_READ,
				    MAP_SHARED|MAP_ANONYMOUS, -1, 0)
					!= MAP_FAILED)
				;
			if (raise(SIGSTOP) != 0)
				tst_brkm(TBROK|TERRNO, tst_exit, "raise");
			exit(0);
		default:
			break;
		}
		/* wait child done mmap and stop */
		if (waitpid(pid, &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, cleanup, "child did not stopped");

		map_count = count_maps(pid);
		if (map_count == max_maps)
			tst_resm(TPASS, "%ld map entries in total "
					"as expected.", max_maps);
		else
			tst_resm(TFAIL, "%ld map entries in total, but "
					"expected %ld entries", map_count,
					max_maps);

		/* make child continue to exit */
		if (kill(pid, SIGCONT) != 0)
			tst_brkm(TBROK|TERRNO, cleanup, "kill");
		if (waitpid(pid, &status, 0) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");

		max_maps *= 10;
	}
}
