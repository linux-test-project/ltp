/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 * NAME
 *	gettimeofday01.c
 *
 * DESCRIPTION
 *	Testcase to check that gettimeofday(2) sets errno to EFAULT.
 *
 * ALGORITHM
 *	Call gettimeofday() with an invalid buffer, and expect EFAULT to be
 *	set in errno.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <sys/time.h>
#include <errno.h>
#include "test.h"
#include <unistd.h>
#include "lapi/syscalls.h"

char *TCID = "gettimeofday01";
int TST_TOTAL = 1;

#if !defined UCLINUX

void cleanup(void);
void setup(void);

int main(int ac, char **av)
{
	int lc;
	int ret;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(ltp_syscall(__NR_gettimeofday, (void *)-1, (void *)-1));

		/* gettimeofday returns an int, so we need to turn the long
		 * TEST_RETURN into an int to test with */
		ret = TEST_RETURN;
		if (ret != -1) {
			tst_resm(TFAIL,
				 "call succeeded unexpectedly (got back %i, wanted -1)",
				 ret);
			continue;
		}

		if (TEST_ERRNO == EFAULT)
			tst_resm(TPASS,
				 "gettimeofday(2) set the errno EFAULT correctly");
		else
			tst_resm(TFAIL,
				 "gettimeofday(2) didn't set errno to EFAULT, errno=%i (%s)",
				 errno, strerror(errno));
	}

	cleanup();
	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
#else

int main(void)
{
	tst_brkm(TCONF, "gettimeofday EFAULT check disabled on uClinux");
}

#endif
