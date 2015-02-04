/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test.h"
#include "migrate_pages_common.h"

void set_bit(unsigned long *b, unsigned int n, unsigned int v)
{
	if (v)
		b[n / bitsperlong] |= 1UL << (n % bitsperlong);
	else
		b[n / bitsperlong] &= ~(1UL << (n % bitsperlong));
}

int check_ret(long expected_ret)
{
	if (expected_ret == TEST_RETURN) {
		tst_resm(TPASS, "expected ret success: "
			 "returned value = %ld", TEST_RETURN);
		return 0;
	} else
		tst_resm(TFAIL, "unexpected failure - "
			 "returned value = %ld, expected: %ld",
			 TEST_RETURN, expected_ret);
	return 1;
}

int check_errno(long expected_errno)
{
	if (TEST_ERRNO == expected_errno) {
		tst_resm(TPASS | TTERRNO, "expected failure");
		return 0;
	} else if (TEST_ERRNO == 0)
		tst_resm(TFAIL, "call succeeded unexpectedly");
	else
		tst_resm(TFAIL | TTERRNO, "unexpected failure - "
			 "expected = %ld : %s, actual",
			 expected_errno, strerror(expected_errno));
	return 1;
}
