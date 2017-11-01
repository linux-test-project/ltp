/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* Check for CVE-2016-9604; that keys beginning with "." are disallowed.
 *
 * See commit ee8f844e3c5a73b999edf733df1c529d6503ec2f
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
	.min_kver = "2.6.13",
};
