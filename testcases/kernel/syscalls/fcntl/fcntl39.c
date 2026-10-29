// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 CTERA Networks. All Rights Reserved.
 *
 * Started by Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * Check that dnotify DN_RENAME event is reported only on rename inside same parent.
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/fcntl.h"

#define	TEST_DIR	"test_dir"
#define	TEST_DIR2	"test_dir2"
#define	TEST_FILE	"test_file"

#define TEST_SIG (SIGRTMIN+1)

static int parent_fd, subdir_fd;
static int got_parent_event, got_subdir_event;

static void dnotify_handler(int sig, siginfo_t *si, void *data LTP_ATTRIBUTE_UNUSED)
{
	if (si->si_fd == parent_fd)
		got_parent_event = 1;
	else if (si->si_fd == subdir_fd)
		got_subdir_event = 1;
	else
		tst_brk(TBROK, "Got unexpected signal %d with si_fd %d", sig, si->si_fd);
}

static void setup_dnotify(int fd)
{
	struct sigaction act;

	act.sa_sigaction = dnotify_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(TEST_SIG, &act, NULL);

	TEST(fcntl(fd, F_SETSIG, TEST_SIG));
	if (TST_RET != 0) {
		tst_brk(TBROK, "F_SETSIG failed errno = %d : %s",
			TST_ERR, strerror(TST_ERR));
	}

	TEST(fcntl(fd, F_NOTIFY, DN_RENAME|DN_MULTISHOT));
	if (TST_RET != 0) {
		tst_brk(TBROK, "F_NOTIFY failed errno = %d : %s",
			TST_ERR, strerror(TST_ERR));
	}
}

static void verify_dnotify(void)
{
	parent_fd = SAFE_OPEN(".", O_RDONLY);
	subdir_fd = SAFE_OPEN(TEST_DIR, O_RDONLY);

	/* Watch renames inside ".", but not in and out of "." */
	setup_dnotify(parent_fd);

	/* Also watch for renames inside subdir, but not in and out of subdir */
	setup_dnotify(subdir_fd);

	/* Rename file from "." to subdir should not generate DN_RENAME on either */
	tst_res(TINFO, "Testing no DN_RENAME on rename from parent to subdir");
	SAFE_RENAME(TEST_FILE, TEST_DIR "/" TEST_FILE);

	if (got_parent_event)
		tst_res(TFAIL, "Got unexpected event on parent");
	else
		tst_res(TPASS, "No event on parent as expected");

	if (got_subdir_event)
		tst_res(TFAIL, "Got unexpected event on subdir");
	else
		tst_res(TPASS, "No event on subdir as expected");

	/* Rename subdir itself should generate DN_RENAME on ".", but not on itself */
	tst_res(TINFO, "Testing DN_RENAME on rename of subdir itself");
	SAFE_RENAME(TEST_DIR, TEST_DIR2);

	if (got_parent_event)
		tst_res(TPASS, "Got event on parent as expected");
	else
		tst_res(TFAIL, "Missing event on parent");

	if (got_subdir_event)
		tst_res(TFAIL, "Got unexpected event on subdir");
	else
		tst_res(TPASS, "No event on subdir as expected");

	SAFE_CLOSE(parent_fd);
	SAFE_CLOSE(subdir_fd);

	/* Cleanup before rerun */
	SAFE_RENAME(TEST_DIR2 "/" TEST_FILE, TEST_FILE);
	SAFE_RENAME(TEST_DIR2, TEST_DIR);
	got_parent_event = 0;
	got_subdir_event = 0;
}

static void setup(void)
{
	SAFE_MKDIR(TEST_DIR, 00700);
	SAFE_TOUCH(TEST_FILE, 0666, NULL);
}

static void cleanup(void)
{
	if (parent_fd > 0)
		SAFE_CLOSE(parent_fd);

	if (subdir_fd > 0)
		SAFE_CLOSE(subdir_fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_dnotify,
	.needs_kconfigs = (const char *[]) { "CONFIG_DNOTIFY=y", NULL },
};
