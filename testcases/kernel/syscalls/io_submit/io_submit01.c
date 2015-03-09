/*
 *
 *   Copyright (c) Crackerjack Project., 2007
 *   Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Porting from Crackerjack to LTP is done
   by Masatake YAMATO <yamato@redhat.com> */

#include "config.h"
#include "test.h"

char *TCID = "io_submit01";

int TST_TOTAL = 3;

#ifdef HAVE_LIBAIO_H
#include <libaio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define TESTFILE	"testfile"

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = open(TESTFILE, O_CREAT | O_RDWR, 0755);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "open");
	if (close(fd) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "close");
}

static void check_result(long exp, long act)
{
	if (exp >= 0) {
		if (act == exp)
			tst_resm(TPASS, "expected success - "
				 "returned value = %ld", act);
		else
			tst_resm(TFAIL, "unexpected failure - "
				 "returned value = %ld : %s",
				 act, strerror(-1 * act));
		return;
	}

	/* if return value is expected to be < 0 */
	if (act == exp)
		tst_resm(TPASS, "expected failure - "
			 "returned value = %ld : %s", act, strerror(-1 * act));
	else if (act == 0)
		tst_resm(TFAIL, "call succeeded unexpectedly");
	else
		tst_resm(TFAIL, "unexpected failure - "
			 "returned value = %ld : %s, "
			 "expected value = %ld : %s",
			 act, strerror(-1 * act), exp, strerror(-1 * exp));
}

int main(int argc, char *argv[])
{
	int lc;

	int rval, fd;
	char buf[256];
	struct iocb iocb;
	struct iocb *iocbs[1];
	io_context_t ctx;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/* 1 - EINVAL */
		/* 1.1 - EINVAL: invalid ctx */
		memset(&ctx, 0, sizeof(ctx));
		TEST(io_submit(ctx, 0, NULL));
		check_result(-EINVAL, TEST_RETURN);

		/* 1.2 - EINVAL: invalid nr */
		rval = io_setup(1, &ctx);
		if (rval != 0)
			tst_brkm(TBROK, cleanup, "io_setup failed: %d", rval);
		TEST(io_submit(ctx, -1, NULL));
		check_result(-EINVAL, TEST_RETURN);

		/* 1.3 - EINVAL: uninitialized iocb */
		iocbs[0] = &iocb;

		/* There are multiple checks we can hit with uninitialized
		 * iocb, but with "random" data it's not 100%. Make sure we
		 * fail eventually in opcode check. */
		iocb.aio_lio_opcode = -1;

		TEST(io_submit(ctx, 1, iocbs));
		switch (TEST_RETURN) {
		case -EINVAL:
		case -EBADF:
		case -EFAULT:
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s",
				 TEST_RETURN, strerror(-1 * TEST_RETURN));
			break;
		default:
			tst_resm(TFAIL, "unexpected failure - "
				 "returned value = %ld : %s, "
				 "expected one of -EINVAL, -EBADF, -EFAULT",
				 TEST_RETURN, strerror(-1 * TEST_RETURN));
		}

		/* 2 - EFAULT: iocb points to invalid data */
		TEST(io_submit(ctx, 1, (struct iocb **)-1));
		check_result(-EFAULT, TEST_RETURN);

		/*
		 * 3 - Special case EFAULT or EINVAL (indetermination)
		 *
		 * The errno depends on the per architecture implementation
		 * of io_submit. On the architecture using compat_sys_io_submit
		 * as its implementation, errno is set to -EINVAL.
		 */
		TEST(io_submit(ctx, -1, (struct iocb **)-1));
		if (TEST_RETURN == 0)
			tst_resm(TFAIL, "call succeeded unexpectedly");
		else if (TEST_RETURN == -EFAULT || TEST_RETURN == -EINVAL)
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s",
				 TEST_RETURN, strerror(-1 * TEST_RETURN));
		else
			tst_resm(TFAIL, "unexpected failure - "
				 "returned value = %ld : %s, "
				 "expected = %d : %s or %d : %s",
				 TEST_RETURN, strerror(-1 * TEST_RETURN),
				 -EFAULT, strerror(EFAULT),
				 -EINVAL, strerror(EINVAL));

		/*
		 * 4 - EBADF: fd in iocb is invalid
		 */
		io_prep_pread(&iocb, -1, buf, sizeof(buf), 0);
		iocbs[0] = &iocb;
		TEST(io_submit(ctx, 1, iocbs));
		check_result(-EBADF, TEST_RETURN);

		/* 5 - Positive test: nr == 0 */
		TEST(io_submit(ctx, 0, NULL));
		check_result(0, TEST_RETURN);

		/* 6 - Positive test: valid fd */
		fd = open(TESTFILE, O_RDONLY);
		if (fd == -1)
			tst_resm(TBROK | TERRNO, "open");
		io_prep_pread(&iocb, fd, buf, sizeof(buf), 0);
		iocbs[0] = &iocb;
		TEST(io_submit(ctx, 1, iocbs));
		check_result(1, TEST_RETURN);
		if (close(fd) == -1)
			tst_resm(TBROK | TERRNO, "close");

	}
	cleanup();

	tst_exit();
}
#else
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "System doesn't support execution of the test");
}
#endif
