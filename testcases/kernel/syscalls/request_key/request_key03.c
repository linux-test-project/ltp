// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 */

/*
 * Regression test for two related bugs:
 *
 * (1) CVE-2017-15299, fixed by commit 60ff5b2f547a ("KEYS: don't let add_key()
 *     update an uninstantiated key")
 * (2) CVE-2017-15951, fixed by commit 363b02dab09b ("KEYS: Fix race between
 *     updating and finding a negative key")
 *
 * We test for the bugs together because the reproduction steps are essentially
 * the same: repeatedly try to add/update a key with add_key() while requesting
 * it with request_key() in another task.  This reproduces both bugs:
 *
 * For CVE-2017-15299, add_key() has to run while the key being created by
 * request_key() is still in the "uninstantiated" state.  For the "encrypted" or
 * "trusted" key types (not guaranteed to be available) this caused a NULL
 * pointer dereference in encrypted_update() or in trusted_update(),
 * respectively.  For the "user" key type, this caused the WARN_ON() in
 * construct_key() to be hit.
 *
 * For CVE-2017-15951, request_key() has to run while the key is "negatively
 * instantiated" (from a prior request_key()) and is being concurrently changed
 * to "positively instantiated" via add_key() updating it.  This race, which is
 * a bit more difficult to reproduce, caused the task executing request_key() to
 * dereference an invalid pointer in __key_link_begin().
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static char *opt_bug;

static void test_with_key_type(const char *type, const char *payload,
			       int effort)
{
	int i;
	int status;
	pid_t add_key_pid;
	pid_t request_key_pid;
	bool info_only;

	TEST(keyctl(KEYCTL_JOIN_SESSION_KEYRING, NULL));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to join new session keyring");

	TEST(add_key(type, "desc", payload, strlen(payload),
		     KEY_SPEC_SESSION_KEYRING));
	if (TST_RET < 0 && TST_ERR != EINVAL) {
		if (TST_ERR == ENODEV) {
			tst_res(TCONF, "kernel doesn't support key type '%s'",
				type);
			return;
		}
		tst_brk(TBROK | TTERRNO,
			"unexpected error checking whether key type '%s' is supported",
			type);
	}

	/*
	 * Fork a subprocess which repeatedly tries to "add" a key of the given
	 * type.  This actually will try to update the key if it already exists.
	 * Depending on the state of the key, add_key() should either succeed or
	 * fail with one of several errors:
	 *
	 * (1) key didn't exist at all: either add_key() should succeed (if the
	 *     payload is valid), or it should fail with EINVAL (if the payload
	 *     is invalid; this is needed for the "encrypted" and "trusted" key
	 *     types because they have a quirk where the payload syntax differs
	 *     for creating new keys vs. updating existing keys)
	 *
	 * (2) key was negative: add_key() should succeed
	 *
	 * (3) key was uninstantiated: add_key() should wait for the key to be
	 *     negated, then fail with ENOKEY
	 *
	 * For now we also accept EDQUOT because the kernel frees up the keys
	 * quota asynchronously after keys are unlinked.  So it may be hit.
	 */
	add_key_pid = SAFE_FORK();
	if (add_key_pid == 0) {
		for (i = 0; i < 100 * effort; i++) {
			usleep(rand() % 1024);
			TEST(add_key(type, "desc", payload, strlen(payload),
				     KEY_SPEC_SESSION_KEYRING));
			if (TST_RET < 0 && TST_ERR != EINVAL &&
			    TST_ERR != ENOKEY && TST_ERR != EDQUOT) {
				tst_brk(TBROK | TTERRNO,
					"unexpected error adding key of type '%s'",
					type);
			}
			TEST(keyctl(KEYCTL_CLEAR, KEY_SPEC_SESSION_KEYRING));
			if (TST_RET < 0) {
				tst_brk(TBROK | TTERRNO,
					"unable to clear keyring");
			}
		}
		exit(0);
	}

	request_key_pid = SAFE_FORK();
	if (request_key_pid == 0) {
		for (i = 0; i < 5000 * effort; i++) {
			TEST(request_key(type, "desc", "callout_info",
					 KEY_SPEC_SESSION_KEYRING));
			if (TST_RET < 0 && TST_ERR != ENOKEY &&
			    TST_ERR != ENOENT && TST_ERR != EDQUOT) {
				tst_brk(TBROK | TTERRNO,
					"unexpected error requesting key of type '%s'",
					type);
			}
		}
		exit(0);
	}

	/*
	 * Verify that neither the add_key() nor the request_key() process
	 * crashed.  If the add_key() process crashed it is likely due to
	 * CVE-2017-15299, while if the request_key() process crashed it is
	 * likely due to CVE-2017-15951.  If testing for one of the bugs
	 * specifically, only pay attention to the corresponding process.
	 */

	SAFE_WAITPID(add_key_pid, &status, 0);
	info_only = (opt_bug && strcmp(opt_bug, "cve-2017-15299") != 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_res(info_only ? TINFO : TPASS,
			"didn't crash while updating key of type '%s'",
			type);
	} else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(info_only ? TINFO : TFAIL,
			"kernel oops while updating key of type '%s'",
			type);
	} else {
		tst_brk(TBROK, "add_key child %s", tst_strstatus(status));
	}

	SAFE_WAITPID(request_key_pid, &status, 0);
	info_only = (opt_bug && strcmp(opt_bug, "cve-2017-15951") != 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_res(info_only ? TINFO : TPASS,
			"didn't crash while requesting key of type '%s'",
			type);
	} else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(info_only ? TINFO : TFAIL,
			"kernel oops while requesting key of type '%s'",
			type);
	} else {
		tst_brk(TBROK, "request_key child %s", tst_strstatus(status));
	}
}

static void do_test(void)
{
	/*
	 * Briefly test the "encrypted" and/or "trusted" key types when
	 * availaible, mainly to reproduce CVE-2017-15299.
	 */
	test_with_key_type("encrypted", "update user:foo 32", 2);
	test_with_key_type("trusted", "update", 2);

	/*
	 * Test the "user" key type for longer, mainly in order to reproduce
	 * CVE-2017-15951.  However, without the fix for CVE-2017-15299 as well,
	 * WARNs may show up in the kernel log.
	 *
	 * Note: the precise iteration count is arbitrary; it's just intended to
	 * be enough to give a decent chance of reproducing the bug, without
	 * wasting too much time.
	 */
	test_with_key_type("user", "payload", 20);
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"b:", &opt_bug,  "Bug to test for (cve-2017-15299 or cve-2017-15951; default is both)"},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-15299"},
		{"linux-git", "60ff5b2f547a"},
		{"CVE", "2017-15951"},
		{"linux-git", "363b02dab09b"},
		{},
	}
};
