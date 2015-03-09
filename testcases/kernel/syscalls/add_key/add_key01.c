/******************************************************************************
 * Copyright (c) Crackerjack Project., 2007				      *
 *									      *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or	      *
 * (at your option) any later version.					      *
 *									      *
 * This program is distributed in the hope that it will be useful,	      *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of	      *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		      *
 * the GNU General Public License for more details.			      *
 *									      *
 * You should have received a copy of the GNU General Public License	      *
 * along with this program;  if not, write to the Free Software	Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *									      *
 ******************************************************************************/

/*
 * Basic test for the add_key() syscall.
 *
 * History:     Porting from Crackerjack to LTP is done by
 *	      Manas Kumar Nayak maknayak@in.ibm.com>
 */

#include "config.h"

#include <stdio.h>
#include <errno.h>
#ifdef HAVE_LINUX_KEYCTL_H
# include <linux/keyctl.h>
#endif
#include "test.h"
#include "linux_syscall_numbers.h"

char *TCID = "add_key01";
int TST_TOTAL = 1;

#ifdef HAVE_LINUX_KEYCTL_H

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* Call add_key. */
	TEST(ltp_syscall
	     (__NR_add_key, "keyring", "wjkey", NULL, 0,
	      KEY_SPEC_THREAD_KEYRING));
	if (TEST_RETURN == -1)
		tst_resm(TFAIL | TTERRNO, "add_key call failed");
	else
		tst_resm(TPASS, "add_key call succeeded");

	cleanup();
	tst_exit();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "linux/keyctl.h was missing upon compilation.");
}
#endif /* HAVE_LINUX_KEYCTL_H */
