/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "common_timers.h"

static void setup(void);
static void cleanup(void);
static int setup_test(int option);

clockid_t clocks[] = {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	MAX_CLOCKS,
	MAX_CLOCKS + 1,
	CLOCK_REALTIME,
	CLOCK_REALTIME,
	CLOCK_REALTIME,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID
};

int testcases[] = {
	EFAULT,			/* tp bad               */
	EINVAL,			/* CLOCK_MONOTONIC      */
	EINVAL,			/* MAX_CLOCKS           */
	EINVAL,			/* MAX_CLOCKS + 1       */
	EINVAL,			/* Invalid timespec     */
	EINVAL,			/* NSEC_PER_SEC + 1     */
	EPERM,			/* non-root user        */
	EINVAL,			/* PROCESS_CPUTIME_ID	*/
	EINVAL,			/* THREAD_CPUTIME_ID	*/
};

char *TCID = "clock_settime03";
int TST_TOTAL = ARRAY_SIZE(testcases);

char nobody_uid[] = "nobody";
struct passwd *ltpuser;
static struct timespec spec, *temp, saved;

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (setup_test(i) < 0)
				continue;

			TEST(ltp_syscall(__NR_clock_settime, clocks[i], temp));

			/* Change the UID back to root */
			if (i == TST_TOTAL - 1) {
				SAFE_SETEUID(cleanup, 0);
			}

			/* check return code */
			if (TEST_RETURN == -1 && TEST_ERRNO == testcases[i]) {
				tst_resm(TPASS | TTERRNO,
					 "clock_settime(2) got expected "
					 "failure.");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "clock_settime(2) failed to produce "
					 "expected error (return code = %ld)",
					 TEST_RETURN);
				/* Restore the clock to its previous state. */
				if (TEST_RETURN == 0) {
					if (ltp_syscall(__NR_clock_settime,
						    CLOCK_REALTIME,
						    &saved) < 0) {
						tst_resm(TWARN | TERRNO,
							 "FATAL: could not set "
							 "the clock!");
					}
				}
			}

		}

	}

	cleanup();
	tst_exit();
}

static int setup_test(int option)
{
	/* valid timespec */
	spec = saved;
	temp = &spec;

	/* error sceanrios */
	switch (option) {
	case 0:
		/* Make tp argument bad pointer */
		temp = (struct timespec *)-1;
		break;
	case 4:
		/* Make the parameter of timespec invalid */
		spec.tv_nsec = -1;
		break;
	case 5:
		/* Make the parameter of timespec invalid */
		spec.tv_nsec = NSEC_PER_SEC + 1;
		break;
	case 6:
		/* change the User to non-root */
		spec.tv_nsec = 0;
		if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
			tst_resm(TWARN, "user \"nobody\" not present; "
				 "skipping test");
			return -1;
		}
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_resm(TWARN | TERRNO,
				 "seteuid failed to set the effective "
				 "uid to %d (nobody)", ltpuser->pw_uid);
			return -1;
		}
		break;
	}
	return 0;
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	if (ltp_syscall(__NR_clock_gettime, CLOCK_REALTIME, &saved) < 0)
		tst_brkm(TBROK, NULL, "Clock gettime failed");

	spec.tv_sec = 1;
	spec.tv_nsec = 0;

	TEST_PAUSE;
}

static void cleanup(void)
{
}
