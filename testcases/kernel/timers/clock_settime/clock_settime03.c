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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>

#include "test.h"
#include "usctest.h"
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
	EFAULT,	/* tp bad		*/
	EINVAL,	/* CLOCK_MONOTONIC	*/
	EINVAL,	/* MAX_CLOCKS		*/
	EINVAL,	/* MAX_CLOCKS + 1	*/
	EINVAL,	/* Invalid timespec	*/
	EINVAL,	/* NSEC_PER_SEC + 1	*/
	EPERM,	/* non-root user	*/
	0,
	0,
};

char *TCID = "clock_settime03";
int TST_TOTAL = sizeof(testcases) / sizeof(*testcases);

static int exp_enos[] = { EINVAL, EFAULT, EPERM, 0 };
char nobody_uid[] = "nobody";
struct passwd *ltpuser;
static struct timespec spec, *temp, saved;

int main(int ac, char **av)
{
	int lc, i;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);


	/* PROCESS_CPUTIME_ID & THREAD_CPUTIME_ID are not supported on
	 * kernel versions lower than 2.6.12 and changed back in 2.6.38
	 */
	if ((tst_kvercmp(2, 6, 12)) < 0 || (tst_kvercmp(2, 6, 38)) >= 0) {
		testcases[7] = EINVAL;
		testcases[8] = EINVAL;
	} else {
		testcases[7] = EFAULT;
		testcases[8] = EFAULT;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (setup_test(i) < 0)
				continue;

			TEST(syscall(__NR_clock_settime, clocks[i], temp));

			/* Change the UID back to root */
			if (i == TST_TOTAL - 1) {
				if (seteuid(0) == -1) {
					tst_brkm(TBROK | TERRNO, cleanup,
						"Failed to set the effective "
						"uid to root");
				}
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
					if (syscall(__NR_clock_settime,
						CLOCK_REALTIME,	&saved) < 0) {
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
		temp = (struct timespec *) -1;
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
				"uid to %d (nobody)",
				ltpuser->pw_uid);
			return -1;
		}
		break;
	case 7:
	case 8:
		/* Make tp argument bad pointer */
		if (tst_kvercmp(2, 6, 12) >= 0)
			temp = (struct timespec *) -1;
	}
	return 0;
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	if (syscall(__NR_clock_gettime, CLOCK_REALTIME, &saved) < 0)
		tst_brkm(TBROK, NULL, "Clock gettime failed");

	TEST_EXP_ENOS(exp_enos);
	spec.tv_sec = 1;
	spec.tv_nsec = 0;

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
