// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 * Copyright (c) Linux Test Project, 2018-2024
 */

/*\
 * [Description]
 *
 * Regression test for commit 37863c43b2c6 ("KEYS: prevent KEYCTL_READ on
 * negative key").  This is CVE-2017-12192.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void try_to_read_negative_key(void)
{
	key_serial_t key_id;
	char buffer[128];

	/*
	 * Create a negatively instantiated key of the "user" key type.  This
	 * key type is chosen because it has a ->read() method (which makes the
	 * bug reachable) and is available whenever CONFIG_KEYS is enabled.
	 *
	 * request_key() will result in the creation of a negative key provided
	 * that /sbin/request-key isn't configured to positively instantiate the
	 * key, based on the provided type, description, and callout_info.  If
	 * /sbin/request-key doesn't exist, errno will be ENOENT; while if it
	 * does exist and we specify some random unprefixed description, errno
	 * should be ENOKEY (since /sbin/request-key should not be configured to
	 * instantiate random user keys).  In either case a negative key should
	 * be created and we can continue on with the test.  Negative keys last
	 * for 60 seconds so there should be plenty of time for the test.
	 */
	TEST(request_key("user", "description", "callout_info",
			 KEY_SPEC_PROCESS_KEYRING));
	if (TST_RET != -1)
		tst_brk(TBROK, "request_key() unexpectedly succeeded");

	if (TST_ERR != ENOKEY && TST_ERR != ENOENT) {
		tst_brk(TBROK | TTERRNO,
			"request_key() failed with unexpected error");
	}

	/* Get the ID of the negative key by reading the keyring */
	TEST(keyctl(KEYCTL_READ, KEY_SPEC_PROCESS_KEYRING,
		    &key_id, sizeof(key_id)));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_READ unexpectedly failed");
	if (TST_RET != sizeof(key_id)) {
		tst_brk(TBROK, "KEYCTL_READ returned %ld but expected %zu",
			TST_RET, sizeof(key_id));
	}

	/*
	 * Now try to read the negative key.  Unpatched kernels will oops trying
	 * to read from memory address 0x00000000ffffff92.
	 */
	tst_res(TINFO, "trying to read from the negative key...");
	TEST(keyctl(KEYCTL_READ, key_id, buffer, sizeof(buffer)));
	if (TST_RET != -1) {
		tst_brk(TFAIL,
			"KEYCTL_READ on negative key unexpectedly succeeded");
	}
	if (TST_ERR != ENOKEY) {
		tst_brk(TFAIL | TTERRNO,
			"KEYCTL_READ on negative key failed with unexpected error");
	}
	tst_res(TPASS,
		"KEYCTL_READ on negative key expectedly failed with ENOKEY");
}

static void do_test(void)
{
	int status;

	if (SAFE_FORK() == 0) {
		try_to_read_negative_key();
		return;
	}

	SAFE_WAIT(&status);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_res(TPASS, "didn't crash while reading from negative key");
		return;
	}

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(TFAIL, "reading from negative key caused kernel oops");
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == TCONF)
		tst_brk(TCONF, "syscall not implemented");

	tst_brk(TBROK, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-12192"},
		{"linux-git", "37863c43b2c6"},
		{}
	}
};
