/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 * access(2) test for errno(s) EFAULT.
 */

#include <errno.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

static void setup(void);
static void cleanup(void);

char *TCID = "access03";
int TST_TOTAL = 8;

int exp_enos[] = { EFAULT, 0 };	/* List must end with 0 */

/* XXX (garrcoop): uh, this isn't a bad address yo. */
static void *low_addr;
static void *high_addr;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;
	char *msg;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set the expected errnos. */
	TEST_EXP_ENOS(exp_enos);

#define TEST_ACCESS(addr, mode) \
{ \
	if (access(low_addr, mode) == -1) { \
		if (errno == EFAULT) { \
			tst_resm(TPASS, \
			    "access(%p, %s) failed as expected with EFAULT", \
			    addr, #mode); \
		} else { \
			tst_resm(TFAIL|TERRNO, \
			    "access(%p, %s) failed unexpectedly; " \
			    "expected (EFAULT)", addr, #mode); \
		} \
	} else { \
		tst_resm(TFAIL, \
		    "access(%p, %s) succeeded unexpectedly", addr, #mode); \
	} \
}

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST_ACCESS(low_addr, R_OK);
		TEST_ACCESS(low_addr, W_OK);
		TEST_ACCESS(low_addr, X_OK);
		TEST_ACCESS(low_addr, F_OK);

		TEST_ACCESS(high_addr, R_OK);
		TEST_ACCESS(high_addr, W_OK);
		TEST_ACCESS(high_addr, X_OK);
		TEST_ACCESS(high_addr, F_OK);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	low_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (low_addr == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, NULL, "mmap failed");

	high_addr = get_high_address();
	if (high_addr == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "get_high_address failed");
	high_addr++;

	tst_tmpdir();
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}

#else

int main()
{
	tst_brkm(TCONF, NULL, "test not available on UCLINUX");
}

#endif /* if !defined(UCLINUX) */
