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
 * While most applications need less than a thousand maps, and the tunable
 * default value is 65530.
 * When the value is too large or too little, the system would hang, so I
 * choose four special value to test, which can coverage most situation:
 * a) Default value / 100
 * b) Default value / 10
 * c) Default value
 * d) Default value * 10
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

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"
#include "../include/mem.h"

#define MAX_MAP_COUNT_DEFAULT 65530L

char *TCID = "max_map_count";
int TST_TOTAL = 1;
static long old_max_map_count;

static long count_maps(void);
static void max_map_count_test(void);

int main(int argc, char *argv[])
{
	char *msg;
	int lc;

	setup();
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR -%s ", msg);

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

static long count_maps(void)
{
	FILE *fp;
	int ret;
	size_t len;
	char *line = NULL;
	char buf[BUFSIZ];
	long map_count = 0;

	while (mmap(NULL, 1, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1,
		    (off_t)NULL) != MAP_FAILED) ;

	ret = sprintf(buf, "/proc/%d/maps", getpid());
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sprintf");

	if ((fp = fopen(buf, "r")) == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "fopen %s", buf);
	while (getline(&line, &len, fp) != -1)
		map_count++;

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

	tst_resm(TINFO, "Starting max_map_count test...");
	max_maps = MAX_MAP_COUNT_DEFAULT / 1000;
	for (i = 0; i < 4; i++) {
		max_maps *= 10;
		set_sys_tune("max_map_count", max_maps, 1);
		fflush(stdout);

		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			map_count = count_maps();

			/* Except VDS and VSYSCALL special maps */
			map_count -= 2;
			printf("Child[%d] generated %ld map entries.\n",
			       getpid(), map_count);
			if (map_count > max_maps)
				tst_resm(TFAIL, "The actual map entries"
					 " > max_map_count");
			exit(0);
		default:
			if (waitpid(pid, &status, 0) == -1)
				tst_brkm(TBROK | TERRNO, cleanup, "waitpid");

			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
				if (WIFSIGNALED(status)
				    && WTERMSIG(status) == SIGKILL)
					printf("Child[%d] is killed by "
					       "SIGKILL signal.\n", pid);
				else
					tst_resm(TFAIL, "Child[%d] "
						 "failed unexpectedly", pid);
			}
		}
	}
}
