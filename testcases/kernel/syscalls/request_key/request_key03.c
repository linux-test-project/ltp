// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 * Copyright (c) Linux Test Project, 2018-2024
 */

/*\
 * [Description]
 *
 * Regression test for two related bugs:
 *
 * 1. CVE-2017-15299, fixed by commit 60ff5b2f547a ("KEYS: don't let add_key()
 *    update an uninstantiated key")
 * 2. CVE-2017-15951, fixed by commit 363b02dab09b ("KEYS: Fix race between
 *    updating and finding a negative key")
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

static struct test_case {
	const char *type;
	const char *payload;
	int effort;
} testcase_list[] = {
	/*
	 * Briefly test the "encrypted" and/or "trusted" key types when
	 * availaible, mainly to reproduce CVE-2017-15299.
	 */
	{"encrypted", "update user:foo 32", 2},
	{"trusted", "update", 2},

	/*
	 * Test the "user" key type for longer, mainly in order to reproduce
	 * CVE-2017-15951.  However, without the fix for CVE-2017-15299 as well,
	 * WARNs may show up in the kernel log.
	 *
	 * Note: the precise iteration count is arbitrary; it's just intended to
	 * be enough to give a decent chance of reproducing the bug, without
	 * wasting too much time.
	 */
	{"user", "payload", 20},
};

static char *opt_bug;

static void run_child_add(const char *type, const char *payload, int effort)
{
	int i;

	/*
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
	for (i = 0; i < 100 * effort; i++) {
		usleep(rand() % 1024);
		TEST(add_key(type, "desc", payload, strlen(payload),
			KEY_SPEC_SESSION_KEYRING));
		if (TST_RET < 0 && TST_ERR != EINVAL && TST_ERR != ENOKEY &&
			TST_ERR != EDQUOT) {
			tst_brk(TBROK | TTERRNO,
				"unexpected error adding key of type '%s'",
				type);
		}

		TEST(keyctl(KEYCTL_CLEAR, KEY_SPEC_SESSION_KEYRING));

		if (TST_RET < 0)
			tst_brk(TBROK | TTERRNO, "unable to clear keyring");

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "add_key() process runtime exceeded");
			break;
		}
	}
}

static void run_child_request(const char *type, int effort)
{
	int i;

	for (i = 0; i < 5000 * effort; i++) {
		TEST(request_key(type, "desc", "callout_info",
			KEY_SPEC_SESSION_KEYRING));
		if (TST_RET < 0 && TST_ERR != ENOKEY && TST_ERR != ENOENT &&
			TST_ERR != EDQUOT) {
			tst_brk(TBROK | TTERRNO,
				"unexpected error requesting key of type '%s'",
				type);
		}

		if (!tst_remaining_runtime()) {
			tst_res(TINFO,
				"request_key() process runtime exceeded");
			break;
		}
	}
}

static void do_test(unsigned int n)
{
	int status;
	pid_t add_key_pid;
	pid_t request_key_pid;
	bool info_only;
	struct test_case *tc = testcase_list + n;

	TEST(keyctl(KEYCTL_JOIN_SESSION_KEYRING, NULL));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to join new session keyring");

	TEST(add_key(tc->type, "desc", tc->payload, strlen(tc->payload),
		     KEY_SPEC_SESSION_KEYRING));
	if (TST_RET < 0 && TST_ERR != EINVAL) {
		if (TST_ERR == ENODEV) {
			tst_res(TCONF, "kernel doesn't support key type '%s'",
				tc->type);
			return;
		}
		tst_brk(TBROK | TTERRNO,
			"unexpected error checking whether key type '%s' is supported",
			tc->type);
	}

	/*
	 * Fork a subprocess which repeatedly tries to "add" a key of the given
	 * type.  This actually will try to update the key if it already exists.
	 */
	add_key_pid = SAFE_FORK();
	if (add_key_pid == 0) {
		run_child_add(tc->type, tc->payload, tc->effort);
		exit(0);
	}

	request_key_pid = SAFE_FORK();
	if (request_key_pid == 0) {
		run_child_request(tc->type, tc->effort);
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
			tc->type);
	} else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(info_only ? TINFO : TFAIL,
			"kernel oops while updating key of type '%s'",
			tc->type);
	} else {
		tst_brk(TBROK, "add_key child %s", tst_strstatus(status));
	}

	SAFE_WAITPID(request_key_pid, &status, 0);
	info_only = (opt_bug && strcmp(opt_bug, "cve-2017-15951") != 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_res(info_only ? TINFO : TPASS,
			"didn't crash while requesting key of type '%s'",
			tc->type);
	} else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(info_only ? TINFO : TFAIL,
			"kernel oops while requesting key of type '%s'",
			tc->type);
	} else {
		tst_brk(TBROK, "request_key child %s", tst_strstatus(status));
	}
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(testcase_list),
	.forks_child = 1,
	.runtime = 20,
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
