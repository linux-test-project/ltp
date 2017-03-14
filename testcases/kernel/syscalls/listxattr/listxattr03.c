/*
*  Copyright (c) 2016 RT-RK Institute for Computer Based Systems
*  Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>
*
*  This program is free software;  you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY;  without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
*  the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.
*/

/*
* Test Name: listxattr03
*
* Description:
* An empty buffer of size zero can return the current size of the list
* of extended attribute names, which can be used to estimate a suitable buffer.
*/

#include "config.h"
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define SECURITY_KEY	"security.ltptest"
#define VALUE	"test"
#define VALUE_SIZE	(sizeof(VALUE) - 1)

static const char * const filename[] = {"testfile1", "testfile2"};

static int check_suitable_buf(const char *name, long size)
{
	int n;
	char buf[size];

	n = listxattr(name, buf, sizeof(buf));

	return n != -1;
}

static void verify_listxattr(unsigned int n)
{
	const char *name = filename[n];

	TEST(listxattr(name, NULL, 0));
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "listxattr() failed");
		return;
	}

	if (check_suitable_buf(name, TEST_RETURN))
		tst_res(TPASS, "listxattr() succeed with suitable buffer");
	else
		tst_res(TFAIL, "listxattr() failed with small buffer");
}

static void setup(void)
{
	SAFE_TOUCH(filename[0], 0644, NULL);

	SAFE_TOUCH(filename[1], 0644, NULL);

	SAFE_SETXATTR(filename[1], SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_listxattr,
	.tcnt = ARRAY_SIZE(filename),
	.setup = setup,
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
