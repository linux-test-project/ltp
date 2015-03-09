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
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *                                                                            *
 ******************************************************************************/
/*
 * Description: This tests the keyctl() syscall
 *		Manipulate the kernel's key management facility
 *
 * History:     Porting from Crackerjack to LTP is done by
 *	      Manas Kumar Nayak maknayak@in.ibm.com>
 */

#include "config.h"
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#ifdef HAVE_LINUX_KEYCTL_H
# include <linux/keyctl.h>
#endif

#include "test.h"
#include "linux_syscall_numbers.h"

char *TCID = "keyctl01";
int testno;
int TST_TOTAL = 2;

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
	int ret;
	int lc;
	int32_t ne_key;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (testno = 1; testno < TST_TOTAL; ++testno) {

			/* Call keyctl() and ask for a keyring's ID. */
			ret = ltp_syscall(__NR_keyctl, KEYCTL_GET_KEYRING_ID,
				      KEY_SPEC_USER_SESSION_KEYRING);
			if (ret != -1) {
				tst_resm(TPASS,
					 "KEYCTL_GET_KEYRING_ID succeeded");
			} else {
				tst_resm(TFAIL | TERRNO,
					 "KEYCTL_GET_KEYRING_ID");
			}

			for (ne_key = INT32_MAX; ne_key > INT32_MIN; ne_key--) {
				ret = ltp_syscall(__NR_keyctl, KEYCTL_READ,
					ne_key);
				if (ret == -1 && errno == ENOKEY)
					break;
			}

			/* Call keyctl. */
			ret = ltp_syscall(__NR_keyctl, KEYCTL_REVOKE, ne_key);
			if (ret != -1) {
				tst_resm(TFAIL | TERRNO,
					 "KEYCTL_REVOKE succeeded unexpectedly");
			} else {
				/* Check for the correct error num. */
				if (errno == ENOKEY) {
					tst_resm(TPASS | TERRNO,
						 "KEYCTL_REVOKE got expected "
						 "errno");
				} else {
					tst_resm(TFAIL | TERRNO,
						 "KEYCTL_REVOKE got unexpected "
						 "errno");
				}

			}

		}

	}
	cleanup();
	tst_exit();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "keyctl syscall support not available on system");
}
#endif /* HAVE_LINUX_KEYCTL_H */
