// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 */

/*
 * Regression test for commit ea6789980fda ("assoc_array: Fix a buggy
 * node-splitting case"), or CVE-2017-12193.
 *
 * Reproducing this bug requires adding keys to a keyring in a certain way that
 * triggers a corner case in the kernel's "associative array" implementation,
 * which is the data structure used to hold keys in a keyring, indexed by type
 * and description.
 *
 * Specifically, the root node of a keyring's associative array must be
 * completely filled with keys that all cluster together within the same slot.
 * Then a key must be added which goes in a different slot.  On broken kernels,
 * this caused a NULL pointer dereference in assoc_array_apply_edit().
 *
 * This can be done by carefully crafting key descriptions.  However, an easier
 * way is to just add 16 keyrings and then a non-keyring, since keyrings all go
 * into their own top-level slot.  This test takes the easier approach.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

#define ASSOC_ARRAY_FAN_OUT 16

#define PAYLOAD "payload"

static char *payload;

static void do_test(void)
{
	int status;

	TEST(keyctl(KEYCTL_JOIN_SESSION_KEYRING, NULL));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to join new session keyring");

	if (SAFE_FORK() == 0) {
		char description[32];
		int i;

		for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
			sprintf(description, "keyring%d", i);
			TEST(add_key("keyring", description, NULL, 0,
				     KEY_SPEC_SESSION_KEYRING));
			if (TST_RET < 0) {
				tst_brk(TBROK | TTERRNO,
					"unable to create keyring %d", i);
			}
		}

		TEST(add_key("user", "userkey", payload, sizeof(PAYLOAD),
			     KEY_SPEC_SESSION_KEYRING));
		if (TST_RET < 0)
			tst_brk(TBROK | TTERRNO, "unable to create user key");

		exit(0);
	}

	SAFE_WAIT(&status);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		tst_res(TPASS, "didn't crash while filling keyring");
	else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL)
		tst_res(TFAIL, "kernel oops while filling keyring");
	else
		tst_brk(TBROK, "Child %s", tst_strstatus(status));
}

static void setup(void)
{
	payload = tst_strdup(PAYLOAD);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = do_test,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-12193"},
		{"linux-git", "ea6789980fda"},
		{}
	}
};
