/*
 * This is a reproducer of CVE-2011-0999, which fixed by mainline commit
 * a7d6e4ecdb7648478ddec76d30d87d03d6e22b31:
 *
 * "Transparent hugepages can only be created if rmap is fully
 * functional. So we must prevent hugepages to be created while
 * is_vma_temporary_stack() is true."
 *
 * It will cause a panic something like this, if the patch didn't get
 * applied:
 *
 * kernel BUG at mm/huge_memory.c:1260!
 * invalid opcode: 0000 [#1] SMP
 * last sysfs file: /sys/devices/system/cpu/cpu23/cache/index2/shared_cpu_map
 * ....
 *
 * Copyright (C) 2011  Red Hat, Inc.
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
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"

char *TCID = "thp01";
int TST_TOTAL = 1;

#define ARRAY_SZ	256

static int ps;
static long length;
static char *array[ARRAY_SZ];
static char *arg;
static struct rlimit rl = {
	.rlim_cur = RLIM_INFINITY,
	.rlim_max = RLIM_INFINITY,
};

static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int i, lc, st;
	pid_t pid;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			memset(arg, 'c', length - 1);
			arg[length - 1] = '\0';
			array[0] = "true";
			for (i = 1; i < ARRAY_SZ - 1; i++)
				array[i] = arg;
			array[ARRAY_SZ - 1] = NULL;
			if (setrlimit(RLIMIT_STACK, &rl) == -1) {
				perror("setrlimit");
				exit(1);
			}
			if (execvp("true", array) == -1) {
				perror("execvp");
				exit(1);
			}
		default:
			if (waitpid(pid, &st, 0) == -1)
				tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
			if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
				tst_brkm(TBROK, cleanup,
					 "child exited abnormally");
		}
	}
	tst_resm(TPASS, "system didn't crash, pass.");
	cleanup();
	tst_exit();
}

static void setup(void)
{
	ps = sysconf(_SC_PAGESIZE);
	length = 32 * ps;
	arg = malloc(length);
	if (arg == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "malloc");

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

static void cleanup(void)
{
}
