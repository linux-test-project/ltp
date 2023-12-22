/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef GETCWD_H
#define GETCWD_H

#include <stdint.h>
#include "config.h"
#include "lapi/syscalls.h"

static inline void
check_getcwd(char *buf, size_t size, int exp_err)
{
	char *res;

	errno = 0;
	res = getcwd(buf, size);
	TST_ERR = errno;
	if (res) {
		tst_res(TFAIL, "getcwd() succeeded unexpectedly");
		return;
	}

	if (TST_ERR != exp_err) {
		tst_res(TFAIL | TTERRNO, "getcwd() failed unexpectedly, expected %s",
				tst_strerrno(exp_err));
		return;
	}

	tst_res(TPASS | TTERRNO, "getcwd() failed as expected");
}

static inline void
tst_getcwd(char *buf, size_t size, int exp_err, int exp_err2)
{
	switch (tst_variant) {
	case 0:
		TST_EXP_FAIL2(tst_syscall(__NR_getcwd, buf, size), exp_err);
		break;
	case 1:
#ifdef __GLIBC__
		check_getcwd(buf, size, exp_err2);
#endif
		break;
	}
}

static inline void
getcwd_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing getcwd with raw syscall");
		break;
	case 1:
		tst_res(TINFO, "Testing getcwd with wrap syscall");
		break;
	}
}

#define TEST_VARIANTS 2

#endif /* GETCWD_H */
