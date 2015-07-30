/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	03/2001 - Written by Wayne Boyer
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
 *	setpriority01 - set the priority for the test process lower.
 */

#include "test.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

static void cleanup(void);
static void setpriority_verify(const int);
static void setup(void);

char *TCID = "setpriority01";
int TST_TOTAL = 40;

int main(int ac, char **av)
{
	int lc;
	int new_val;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (new_val = -20; new_val < 20; new_val++)
			setpriority_verify(new_val);
	}

	cleanup();
	tst_exit();

}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void setpriority_verify(const int new_prio)
{
	int priority;
	TEST(setpriority(PRIO_PROCESS, 0, new_prio));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "setpriority(%d) failed", new_prio);
		return;
	}

	priority = getpriority(PRIO_PROCESS, 0);
	if (errno == -1) {
		tst_brkm(TBROK, cleanup,
			 "getpriority call failed - errno = %d - %s", errno,
			 strerror(errno));
	}

	if (priority == new_prio) {
		tst_resm(TPASS, "setpriority(%d) succeeded", new_prio);
	} else {
		tst_resm(TFAIL,
			 "current priority-%d and new priority-%d do not match",
			 priority, new_prio);
	}
}

static void cleanup(void)
{
}
