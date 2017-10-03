/*
 * getrusage03 - test ru_maxrss behaviors in struct rusage
 *
 * This test program is backported from upstream commit:
 * 1f10206cf8e945220f7220a809d8bfc15c21f9a5, which fills ru_maxrss
 * value in struct rusage according to rss hiwater mark. To make sure
 * this feature works correctly, a series of tests are executed in
 * this program.
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
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "getrusage03";
int TST_TOTAL = 1;

#define DELTA_MAX	10240

static struct rusage ru;
static long maxrss_init;
static int retval, status;
static pid_t pid;

static void inherit_fork(void);
static void inherit_fork2(void);
static void fork_malloc(void);
static void grandchild_maxrss(void);
static void zombie(void);
static void sig_ign(void);
static void exec_without_fork(void);
static void check_return(int status, char *pass_msg, char *fail_msg);
static int is_in_delta(long value);
static void consume(int mega);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		tst_resm(TINFO, "allocate 100MB");
		consume(100);

		inherit_fork();
		inherit_fork2();
		fork_malloc();
		grandchild_maxrss();
		zombie();
		sig_ign();
		exec_without_fork();
	}
	cleanup();
	tst_exit();
}

/* Testcase #01: fork inherit
 * expect: initial.self ~= child.self */
static void inherit_fork(void)
{
	tst_resm(TINFO, "Testcase #01: fork inherit");

	SAFE_GETRUSAGE(cleanup, RUSAGE_SELF, &ru);
	tst_resm(TINFO, "initial.self = %ld", ru.ru_maxrss);

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork #1");
	case 0:
		maxrss_init = ru.ru_maxrss;
		SAFE_GETRUSAGE(cleanup, RUSAGE_SELF, &ru);
		tst_resm(TINFO, "child.self = %ld", ru.ru_maxrss);
		exit(is_in_delta(maxrss_init - ru.ru_maxrss));
	default:
		break;
	}

	SAFE_WAITPID(cleanup, pid, &status, WUNTRACED | WCONTINUED);
	check_return(WEXITSTATUS(status), "initial.self ~= child.self",
		     "initial.self !~= child.self");
}

/* Testcase #02: fork inherit (cont.)
 * expect: initial.children ~= 100MB, child.children = 0 */
static void inherit_fork2(void)
{
	tst_resm(TINFO, "Testcase #02: fork inherit(cont.)");

	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "initial.children = %ld", ru.ru_maxrss);
	if (is_in_delta(ru.ru_maxrss - 102400))
		tst_resm(TPASS, "initial.children ~= 100MB");
	else
		tst_resm(TFAIL, "initial.children !~= 100MB");

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork #2");
	case 0:
		SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
		tst_resm(TINFO, "child.children = %ld", ru.ru_maxrss);
		exit(ru.ru_maxrss == 0);
	default:
		break;
	}

	SAFE_WAITPID(cleanup, pid, &status, WUNTRACED | WCONTINUED);
	check_return(WEXITSTATUS(status), "child.children == 0",
		     "child.children != 0");
}

/* Testcase #03: fork + malloc
 * expect: initial.self + 50MB ~= child.self */
static void fork_malloc(void)
{
	tst_resm(TINFO, "Testcase #03: fork + malloc");

	SAFE_GETRUSAGE(cleanup, RUSAGE_SELF, &ru);
	tst_resm(TINFO, "initial.self = %ld", ru.ru_maxrss);

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork #3");
	case 0:
		maxrss_init = ru.ru_maxrss;
		tst_resm(TINFO, "child allocate +50MB");
		consume(50);
		SAFE_GETRUSAGE(cleanup, RUSAGE_SELF, &ru);
		tst_resm(TINFO, "child.self = %ld", ru.ru_maxrss);
		exit(is_in_delta(maxrss_init + 51200 - ru.ru_maxrss));
	default:
		break;
	}

	SAFE_WAITPID(cleanup, pid, &status, WUNTRACED | WCONTINUED);
	check_return(WEXITSTATUS(status), "initial.self + 50MB ~= child.self",
		     "initial.self + 50MB !~= child.self");
}

/* Testcase #04: grandchild maxrss
 * expect: post_wait.children ~= 300MB */
static void grandchild_maxrss(void)
{
	tst_resm(TINFO, "Testcase #04: grandchild maxrss");

	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "initial.children = %ld", ru.ru_maxrss);

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork #4");
	case 0:
		retval = system("getrusage03_child -g 300");
		if ((WIFEXITED(retval) && WEXITSTATUS(retval) != 0))
			tst_brkm(TBROK | TERRNO, cleanup, "system");
		exit(0);
	default:
		break;
	}

	SAFE_WAITPID(cleanup, pid, &status, WUNTRACED | WCONTINUED);
	if (WEXITSTATUS(status) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "child exit status is not 0");

	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "post_wait.children = %ld", ru.ru_maxrss);
	if (is_in_delta(ru.ru_maxrss - 307200))
		tst_resm(TPASS, "child.children ~= 300MB");
	else
		tst_resm(TFAIL, "child.children !~= 300MB");
}

/* Testcase #05: zombie
 * expect: initial ~= pre_wait, post_wait ~= 400MB */
static void zombie(void)
{
	tst_resm(TINFO, "Testcase #05: zombie");

	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "initial.children = %ld", ru.ru_maxrss);
	maxrss_init = ru.ru_maxrss;

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK, cleanup, "fork #5");
	case 0:
		retval = system("getrusage03_child -n 400");
		if ((WIFEXITED(retval) && WEXITSTATUS(retval) != 0))
			tst_brkm(TBROK | TERRNO, cleanup, "system");
		exit(0);
	default:
		break;
	}

	sleep(1);		/* children become zombie */
	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "pre_wait.children = %ld", ru.ru_maxrss);
	if (is_in_delta(ru.ru_maxrss - maxrss_init))
		tst_resm(TPASS, "initial.children ~= pre_wait.children");
	else
		tst_resm(TFAIL, "initial.children !~= pre_wait.children");

	SAFE_WAITPID(cleanup, pid, &status, WUNTRACED | WCONTINUED);
	if (WEXITSTATUS(status) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "child exit status is not 0");

	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "post_wait.children = %ld", ru.ru_maxrss);
	if (is_in_delta(ru.ru_maxrss - 409600))
		tst_resm(TPASS, "post_wait.children ~= 400MB");
	else
		tst_resm(TFAIL, "post_wait.children !~= 400MB");
}

/* Testcase #06: SIG_IGN
 * expect: initial ~= after_zombie */
static void sig_ign(void)
{
	tst_resm(TINFO, "Testcase #06: SIG_IGN");

	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "initial.children = %ld", ru.ru_maxrss);
	signal(SIGCHLD, SIG_IGN);
	maxrss_init = ru.ru_maxrss;

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK, cleanup, "fork #6");
	case 0:
		retval = system("getrusage03_child -n 500");
		if ((WIFEXITED(retval) && WEXITSTATUS(retval) != 0))
			tst_brkm(TBROK | TERRNO, cleanup, "system");
		exit(0);
	default:
		break;
	}

	sleep(1);		/* children become zombie */
	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	tst_resm(TINFO, "after_zombie.children = %ld", ru.ru_maxrss);
	if (is_in_delta(ru.ru_maxrss - maxrss_init))
		tst_resm(TPASS, "initial.children ~= after_zombie.children");
	else
		tst_resm(TFAIL, "initial.children !~= after_zombie.children");
	signal(SIGCHLD, SIG_DFL);
}

/* Testcase #07: exec without fork
 * expect: initial ~= fork */
static void exec_without_fork(void)
{
	char str_maxrss_self[BUFSIZ], str_maxrss_child[BUFSIZ];
	long maxrss_self, maxrss_child;

	tst_resm(TINFO, "Testcase #07: exec without fork");

	SAFE_GETRUSAGE(cleanup, RUSAGE_SELF, &ru);
	maxrss_self = ru.ru_maxrss;
	SAFE_GETRUSAGE(cleanup, RUSAGE_CHILDREN, &ru);
	maxrss_child = ru.ru_maxrss;
	tst_resm(TINFO, "initial.self = %ld, initial.children = %ld",
		 maxrss_self, maxrss_child);

	sprintf(str_maxrss_self, "%ld", maxrss_self);
	sprintf(str_maxrss_child, "%ld", maxrss_child);
	if (execlp("getrusage03_child", "getrusage03_child", "-v",
		   "-s", str_maxrss_self, "-l", str_maxrss_child, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "execlp");
}

static int is_in_delta(long value)
{
	return (value >= -DELTA_MAX && value <= DELTA_MAX);
}

static void check_return(int status, char *pass_msg, char *fail_msg)
{
	switch (status) {
	case 1:
		tst_resm(TPASS, "%s", pass_msg);
		break;
	case 0:
		tst_resm(TFAIL, "%s", fail_msg);
		break;
	default:
		tst_resm(TFAIL, "child exit status is %d", status);
		break;
	}
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
	/* Disable test if the version of the kernel is less than 2.6.32 */
	if ((tst_kvercmp(2, 6, 32)) < 0) {
		tst_resm(TCONF, "This ru_maxrss field is not supported");
		tst_brkm(TCONF, NULL, "before kernel 2.6.32");
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
