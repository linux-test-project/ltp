/*
 * getrusage03_child.c - a child program executed by getrusage03
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
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "getrusage03_child";
int TST_TOTAL = 1;

#define DELTA_MAX	10240

static int opt_consume, opt_grand, opt_show, opt_self, opt_child;
static char *consume_str, *grand_consume_str, *self_str, *child_str;

option_t child_options[] = {
	{"n:", &opt_consume, &consume_str},
	{"g:", &opt_grand, &grand_consume_str},
	{"v", &opt_show, NULL},
	{"s:", &opt_self, &self_str},
	{"l:", &opt_child, &child_str},
	{NULL, NULL, NULL}
};

static void usage(void);
static void consume(int mega);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	pid_t pid;
	long maxrss_self, maxrss_children, delta;
	long consume_nr, grand_consume_nr, self_nr, child_nr;
	struct rusage ru;

	tst_parse_opts(argc, argv, child_options, usage);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if (opt_consume) {
			consume_nr = SAFE_STRTOL(cleanup,
						 consume_str, 0, LONG_MAX);
			tst_resm(TINFO, "child allocate %ldMB", consume_nr);
			consume(consume_nr);
		}

		if (opt_grand) {
			grand_consume_nr = SAFE_STRTOL(cleanup,
						       grand_consume_str, 0,
						       LONG_MAX);
			tst_resm(TINFO, "grandchild allocate %ldMB",
				 grand_consume_nr);
			switch (pid = fork()) {
			case -1:
				tst_brkm(TBROK, cleanup, "fork");
			case 0:
				consume(grand_consume_nr);
				exit(0);
			default:
				break;
			}
			while (waitpid(-1, &pid, WUNTRACED | WCONTINUED) > 0)
				if (WEXITSTATUS(pid) != 0)
					tst_brkm(TBROK | TERRNO, cleanup,
						 "child exit status is not 0");
		}

		if (opt_show) {
			SAFE_GETRUSAGE(cleanup, RUSAGE_SELF, &ru);
			maxrss_self = ru.ru_maxrss;
			SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
			maxrss_children = ru.ru_maxrss;
			tst_resm(TINFO, "exec.self = %ld, exec.children = %ld",
				 maxrss_self, maxrss_children);
			if (opt_self) {
				self_nr = SAFE_STRTOL(cleanup,
						      self_str, 0, LONG_MAX);
				delta = maxrss_self - self_nr;
				if (delta >= -DELTA_MAX && delta <= DELTA_MAX)
					tst_resm(TPASS,
						 "initial.self ~= exec.self");
				else
					tst_resm(TFAIL,
						 "initial.self !~= exec.self");
			}
			if (opt_child) {
				child_nr = SAFE_STRTOL(cleanup,
						       child_str, 0, LONG_MAX);
				delta = maxrss_children - child_nr;
				if (delta >= -DELTA_MAX && delta <= DELTA_MAX)
					tst_resm(TPASS,
						 "initial.children ~= exec.children");
				else
					tst_resm(TFAIL,
						 "initial.children !~= exec.children");
			}
		}
	}

	cleanup();
	tst_exit();
}

static void usage(void)
{
	printf("  -n NUM  consume NUM MB size\n");
	printf("  -g NUM  grandchild consume NUM MB size\n");
	printf("  -v      verbose mode, show rusage info\n");
	printf("  -s NUM  compare rusage_self.maxrss with given NUM\n");
	printf("  -l NUM  compare rusage_children.maxrss with given NUM\n");
}

static void consume(int mega)
{
	size_t sz;
	void *ptr;

	sz = mega * 1024 * 1024;
	ptr = SAFE_MALLOC(cleanup, sz);
	memset(ptr, 0, sz);
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
