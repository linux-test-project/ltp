// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011  Red Hat, Inc.
 * Copyright (C) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Test ru_maxrss behaviors in struct rusage.
 *
 * This test program is backported from upstream commit: 1f10206cf8e9, which
 * fills ru_maxrss value in struct rusage according to rss hiwater mark. To
 * make sure this feature works correctly, a series of tests are executed in
 * this program.
 */

#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"
#include "getrusage03.h"

#define TESTBIN "getrusage03_child"

static struct rusage ru;
static long maxrss_init;

static const char *const resource[] = {
	TESTBIN,
	NULL,
};

static void inherit_fork1(void)
{
	SAFE_GETRUSAGE(RUSAGE_SELF, &ru);
	maxrss_init = ru.ru_maxrss;

	if (!SAFE_FORK()) {
		SAFE_GETRUSAGE(RUSAGE_SELF, &ru);

		if (is_in_delta(maxrss_init - ru.ru_maxrss))
			tst_res(TPASS, "initial.self ~= child.self");
		else
			tst_res(TFAIL, "child.self = %li, expected %li",
				ru.ru_maxrss, maxrss_init);
		exit(0);
	}
	tst_reap_children();
}

static void inherit_fork2(void)
{
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);

	if (is_in_delta(ru.ru_maxrss - 102400))
		tst_res(TPASS, "initial.children ~= 100MB");
	else
		tst_res(TFAIL, "initial.children = %li, expected %i",
			ru.ru_maxrss, 102400);

	if (!SAFE_FORK()) {
		SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);

		if (!ru.ru_maxrss)
			tst_res(TPASS, "child.children == 0");
		else
			tst_res(TFAIL, "child.children = %li, expected %i",
				ru.ru_maxrss, 0);
		exit(0);
	}
	tst_reap_children();
}

static void grandchild_maxrss(void)
{
	if (!SAFE_FORK())
		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "grand_consume", "300", NULL);
	tst_reap_children();
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);

	if (is_in_delta(ru.ru_maxrss - 307200))
		tst_res(TPASS, "child.children ~= 300MB");
	else
		tst_res(TFAIL, "child.children = %li, expected %i",
			ru.ru_maxrss, 307200);
}

static void zombie(void)
{
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	maxrss_init = ru.ru_maxrss;

	pid_t pid = SAFE_FORK();

	if (!pid)
		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "consume", "400", NULL);

	TST_PROCESS_STATE_WAIT(pid, 'Z', 0);
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	if (is_in_delta(ru.ru_maxrss - maxrss_init))
		tst_res(TPASS, "initial.children ~= pre_wait.children");
	else
		tst_res(TFAIL, "pre_wait.children = %li, expected %li",
			ru.ru_maxrss, maxrss_init);

	tst_reap_children();
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	if (is_in_delta(ru.ru_maxrss - 409600))
		tst_res(TPASS, "post_wait.children ~= 400MB");
	else
		tst_res(TFAIL, "post_wait.children = %li, expected %i",
			ru.ru_maxrss, 409600);
}

static void sig_ign(void)
{
	SAFE_SIGNAL(SIGCHLD, SIG_IGN);
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	maxrss_init = ru.ru_maxrss;

	pid_t pid = SAFE_FORK();

	if (!pid)
		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "consume", "500", NULL);

	TST_PROCESS_EXIT_WAIT(pid, 0);
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	if (is_in_delta(ru.ru_maxrss - maxrss_init))
		tst_res(TPASS, "initial.children ~= after_zombie.children");
	else
		tst_res(TFAIL, "after_zombie.children = %li, expected %li",
			ru.ru_maxrss, maxrss_init);

	SAFE_SIGNAL(SIGCHLD, SIG_DFL);
}

static void inherit_exec(void)
{
	if (!SAFE_FORK()) {
		char str_maxrss_self[BUFSIZ], str_maxrss_child[BUFSIZ];

		SAFE_GETRUSAGE(RUSAGE_SELF, &ru);
		sprintf(str_maxrss_self, "%ld", ru.ru_maxrss);
		SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
		sprintf(str_maxrss_child, "%ld", ru.ru_maxrss);

		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "compare", str_maxrss_self, str_maxrss_child, NULL);
	}
	tst_reap_children();
}

void (*testfunc_list[])(void) = {
	inherit_fork1, inherit_fork2, grandchild_maxrss,
	zombie, sig_ign, inherit_exec
};

static void run(unsigned int i)
{
	if (!SAFE_FORK()) {
		if (!SAFE_FORK()) {
			consume_mb(100);
			exit(0);
		}

		SAFE_WAIT(NULL);

		testfunc_list[i]();
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.resource_files = resource,
	.min_kver = "2.6.32",
	.tags = (const struct tst_tag[]) {
		{"linux-git", "1f10206cf8e9"},
		{}
	},
	.test = run,
	.tcnt = ARRAY_SIZE(testfunc_list),
};
