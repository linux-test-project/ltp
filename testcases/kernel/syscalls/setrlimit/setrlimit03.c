/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2017 Xiao Yang <yangx.jy@cn.fujitsu.com>
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

/*
 * DESCRIPTION
 *  1) Test for EPERM when the super-user tries to increase RLIMIT_NOFILE
 *     beyond the system limit.
 *  2) Test for EINVAL when rlim->rlim_cur is greater than rlim->rlim_max.
 */

#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/fs.h>
#include "tst_test.h"

#if !defined(NR_OPEN)
// Taken from definition in /usr/include/linux/fs.h
# define NR_OPEN (1024*1024)
#endif

#define NR_OPEN_PATH "/proc/sys/fs/nr_open"

static struct rlimit rlim1, rlim2;
static unsigned int nr_open = NR_OPEN;

static struct tcase {
	struct rlimit *rlimt;
	int exp_err;
} tcases[] = {
	{&rlim1, EPERM},
	{&rlim2, EINVAL}
};

static void verify_setrlimit(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(setrlimit(RLIMIT_NOFILE, tc->rlimt));
	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly "
			"(nr_open=%u rlim_cur=%lu rlim_max=%lu)", nr_open,
			(unsigned long)(tc->rlimt->rlim_cur),
			(unsigned long)(tc->rlimt->rlim_max));
		return;
	}

	if (TST_ERR != tc->exp_err) {
		tst_res(TFAIL | TTERRNO, "setrlimit() should fail with %s, got",
			tst_strerrno(tc->exp_err));
	} else {
		tst_res(TPASS | TTERRNO, "setrlimit() failed as expected");
	}
}

static void setup(void)
{
	if (!access(NR_OPEN_PATH, F_OK))
		SAFE_FILE_SCANF(NR_OPEN_PATH, "%u", &nr_open);

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &rlim1);
	rlim2.rlim_max = rlim1.rlim_cur;
	rlim2.rlim_cur = rlim1.rlim_max + 1;
	rlim1.rlim_max = nr_open + 1;
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_setrlimit,
	.needs_root = 1
};
