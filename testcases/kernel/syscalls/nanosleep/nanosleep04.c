/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that nanosleep() will fail to suspend the execution
 *  of a process if the specified pause time is invalid.
 *
 * Expected Result:
 *  nanosleep() should return with -1 value and sets errno to EINVAL.
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#include "test.h"

static struct timespec tcases[] = {
	{.tv_sec = -5, .tv_nsec = 9999},
	{.tv_sec = 0, .tv_nsec = 1000000000},
};

char *TCID = "nanosleep04";
int TST_TOTAL = ARRAY_SIZE(tcases);

static void setup(void);

static void verify_nanosleep(struct timespec *tcase)
{
	TEST(nanosleep(tcase, NULL));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "nanosleep() succeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != EINVAL) {
		tst_resm(TFAIL | TTERRNO,
		         "nanosleep() expected failure with EINVAL");
		return;
	}

	tst_resm(TPASS, "nanoslep() failed with EINVAL");
}

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++)
			verify_nanosleep(&tcases[i]);
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	TEST_PAUSE;
}
