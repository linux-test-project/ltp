// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 * Copyright (c) Linux Test Project, 2019-2024
 */

/*\
 * [Description]
 *
 * Test for CVE-2016-9604, checks that keys beginning with "." are disallowed.
 *
 * See commit
 * ee8f844e3c5a ("KEYS: Disallow keyrings beginning with '.' to be joined as session keyrings")
 */

#include <errno.h>
#include "tst_test.h"
#include "lapi/keyctl.h"

void run(void)
{
	if (keyctl_join_session_keyring(".builtin_trusted_keys") == -1) {
		if (errno != EPERM) {
			tst_brk(TBROK | TERRNO,
				"keyctl_join_sessoin_keyring(...)");
		}

		tst_res(TPASS, "Denied access to .builtin_trusted_keys");
	} else {
		tst_res(TFAIL, "Allowed access to .builtin_trusted_keys");
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2016-9604"},
		{"linux-git", "ee8f844e3c5a"},
		{}
	}
};
