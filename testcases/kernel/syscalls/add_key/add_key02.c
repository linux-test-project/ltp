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
 * History:   Porting from Crackerjack to LTP is done by
 *	      Manas Kumar Nayak maknayak@in.ibm.com>
 */

#include "config.h"
#ifdef HAVE_LINUX_KEYCTL_H
# include <linux/keyctl.h>
#endif
#include "tst_test.h"
#include "linux_syscall_numbers.h"

#ifdef HAVE_LINUX_KEYCTL_H
struct tcase {
	char *type;
	char *desc;
	void *payload;
	int plen;
	int exp_errno;
} tcases[] = {
	{"user", "firstkey", NULL, 1, EINVAL}
};
#endif /* HAVE_LINUX_KEYCTL_H */

static void verify_add_key(unsigned int i)
{
#ifdef HAVE_LINUX_KEYCTL_H
	TEST(tst_syscall(__NR_add_key, tcases[i].type, tcases[i].desc,
	                 tcases[i].payload, tcases[i].plen,
	                 KEY_SPEC_USER_KEYRING));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "add_key() passed unexpectedly");
		return;
	}

	if (TEST_ERRNO == tcases[i].exp_errno) {
		tst_res(TPASS | TTERRNO, "add_key() failed expectedly");
		return;
	}

	tst_res(TFAIL | TTERRNO,
	        "add_key() failed unexpectedly, expected %s",
	        tst_strerrno(tcases[i].exp_errno));
#else
	tst_brk(TCONF, "linux/keyctl.h was missing upon compilation.");
#endif /* HAVE_LINUX_KEYCTL_H */
}

static struct tst_test test = {
	.tid = "add_key01",
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_add_key,
};
