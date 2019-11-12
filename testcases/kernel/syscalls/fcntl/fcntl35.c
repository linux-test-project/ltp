// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Description:
 * fcntl(2) manpage states that an unprivileged user could not set the
 * pipe capacity above the limit in /proc/sys/fs/pipe-max-size.  However,
 * an unprivileged user could create a pipe whose initial capacity exceeds
 * the limit.  We add a regression test to check that pipe-max-size caps
 * the initial allocation for a new pipe for unprivileged users, but not
 * for privileged users.
 *
 * This kernel bug has been fixed by:
 *
 * commit 086e774a57fba4695f14383c0818994c0b31da7c
 * Author: Michael Kerrisk (man-pages) <mtk.manpages@gmail.com>
 * Date:   Tue Oct 11 13:53:43 2016 -0700
 *
 * pipe: cap initial pipe capacity according to pipe-max-size limit
 */

#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "lapi/fcntl.h"
#include "tst_test.h"

static int pipe_max_unpriv;
static int test_max_unpriv;
static int test_max_priv;
static struct passwd *pw;
static struct tcase {
	int *exp_sz;
	int exp_usr;
	char *des;
} tcases[] = {
	{&test_max_unpriv, 1, "an unprivileged user"},
	{&test_max_priv, 0, "a privileged user"}
};

static void setup(void)
{
	test_max_unpriv = getpagesize();
	test_max_priv = test_max_unpriv * 16;

	if (!access("/proc/sys/fs/pipe-max-size", F_OK)) {
		SAFE_FILE_SCANF("/proc/sys/fs/pipe-max-size", "%d",
				&pipe_max_unpriv);
		SAFE_FILE_PRINTF("/proc/sys/fs/pipe-max-size", "%d",
				test_max_unpriv);
	} else {
		tst_brk(TCONF, "/proc/sys/fs/pipe-max-size doesn't exist");
	}

	pw = SAFE_GETPWNAM("nobody");
}

static void cleanup(void)
{
	SAFE_FILE_PRINTF("/proc/sys/fs/pipe-max-size", "%d", pipe_max_unpriv);
}

static int verify_pipe_size(int exp_pip_sz, char *desp)
{
	int get_size;
	int fds[2];

	SAFE_PIPE(fds);

	get_size = fcntl(fds[1], F_GETPIPE_SZ);
	if (get_size == -1) {
		tst_res(TFAIL | TERRNO, "fcntl(2) with F_GETPIPE_SZ failed");
		goto end;
	}

	if (get_size != exp_pip_sz) {
		tst_res(TFAIL, "%s init the capacity of a pipe to %d "
			"unexpectedly, expected %d", desp, get_size,
			exp_pip_sz);
	} else {
		tst_res(TPASS, "%s init the capacity of a pipe to %d "
			"successfully", desp, exp_pip_sz);
	}

end:
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);

	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);

	exit(0);
}

static void do_test(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (!SAFE_FORK()) {
		if (tc->exp_usr)
			SAFE_SETUID(pw->pw_uid);

		verify_pipe_size(*tc->exp_sz, tc->des);
	}

	tst_reap_children();
}

static struct tst_test test = {
	.min_kver = "2.6.35",
	.needs_root = 1,
	.forks_child = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "086e774a57fb"},
		{}
	}
};
