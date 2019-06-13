// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) 2016 RT-RK Institute for Computer Based Systems
* Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>
*/

/*
* Test Name: flistxattr02
*
* Description:
* 1) flistxattr(2) fails if the size of the list buffer is too small
* to hold the result.
* 2) flistxattr(2) fails if fd is an invalid file descriptor.
*
* Expected Result:
* 1) flistxattr(2) should return -1 and set errno to ERANGE.
* 2) flistxattr(2) should return -1 and set errno to EBADF.
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

static int fd1;
static int fd2 = -1;

static struct test_case {
	int *fd;
	size_t size;
	int exp_err;
} tc[] = {
	{&fd1, 1, ERANGE},
	{&fd2, 20, EBADF}
};

static void verify_flistxattr(unsigned int n)
{
	struct test_case *t = tc + n;
	char buf[t->size];

	TEST(flistxattr(*t->fd, buf, t->size));
	if (TST_RET != -1) {
		tst_res(TFAIL,
			"flistxattr() succeeded unexpectedly (returned %ld)",
			TST_RET);
		return;
	}

	if (t->exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "flistxattr() failed "
			 "unexpectedlly, expected %s",
			 tst_strerrno(t->exp_err));
	} else {
		tst_res(TPASS | TTERRNO,
			 "flistxattr() failed as expected");
	}
}

static void setup(void)
{
	fd1 = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0644);

	SAFE_FSETXATTR(fd1, SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_flistxattr,
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
	.cleanup = cleanup,
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
