/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, see <http://www.gnu.org/licenses/>.
 */

/* DESCRIPTION
 *	This test will verify that setuid(2) syscall basic functionality.
 *	setuid(2) returns a value of 0 and uid has been set successfully
 *	as a normal or super user.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static void verify_setuid(void)
{
	uid_t uid;

	/* Set the effective user ID to the current real uid */
	uid = getuid();
	UID16_CHECK(uid, setuid);

	TEST(SETUID(uid));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "setuid(%d) failed", uid);
	else
		tst_res(TPASS, "setuid(%d) successfully", uid);
}

static struct tst_test test = {
	.test_all = verify_setuid,
};
