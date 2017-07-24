/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This test verifies sched_setaffinity(2) for all error conditions
 * to occur correctly.
 *
 * sched_setaffinity() returns -1 and sets the error code to:
 *
 * 1) EFAULT, if the supplied memory address is invalid
 * 2) EINVAL, if the mask doesn't contain at least one
 *    permitted cpu
 * 3) ESRCH, if the process whose id is pid could not
 *    be found
 * 4) EPERM, if the calling process doesn't have appropriate
 *    privileges
 */

#define _GNU_SOURCE
#include <errno.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "safe_macros.h"
#include "sched_setaffinity.h"
#include "lapi/syscalls.h"

char *TCID = "sched_setaffinity01";

#define PID_MAX_PATH "/proc/sys/kernel/pid_max"

static cpu_set_t *mask, *emask;
static cpu_set_t *fmask = (void *)-1;
static size_t mask_size, emask_size;
static pid_t self_pid, privileged_pid, free_pid;
static uid_t uid;
static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;
static long ncpus;

static struct test_case_t {
	pid_t *pid;
	size_t *mask_size;
	cpu_set_t **mask;
	int exp_errno;
} test_cases[] = {
	{&self_pid, &mask_size, &fmask, EFAULT},
	{&self_pid, &emask_size, &emask, EINVAL},
	{&free_pid, &mask_size, &mask, ESRCH},
	{&privileged_pid, &mask_size, &mask, EPERM},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void cleanup(void)
{
	if (mask != NULL) {
		CPU_FREE(mask);
		mask = NULL;
	}

	if (emask != NULL) {
		CPU_FREE(emask);
		emask = NULL;
	}

	SAFE_SETEUID(NULL, uid);

	if (privileged_pid > 0) {
		kill(privileged_pid, SIGKILL);
		waitpid(privileged_pid, NULL, 0);
		privileged_pid = 0;
	}
}

static void setup(void)
{
	tst_require_root();
	uid = geteuid();
	ncpus = tst_ncpus_max();

	/* Current mask */
	mask = CPU_ALLOC(ncpus);
	if (mask == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "CPU_ALLOC(%ld) failed",
			ncpus);
	mask_size = CPU_ALLOC_SIZE(ncpus);
	if (sched_getaffinity(0, mask_size, mask) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sched_getaffinity() failed");

	/* Mask with one more cpu than available on the system */
	emask = CPU_ALLOC(ncpus + 1);
	if (emask == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "CPU_ALLOC(%ld) failed",
			ncpus + 1);
	emask_size = CPU_ALLOC_SIZE(ncpus + 1);
	CPU_ZERO_S(emask_size, emask);
	CPU_SET_S(ncpus, emask_size, emask);

	privileged_pid = tst_fork();
	if (privileged_pid == 0) {
		pause();

		exit(0);
	} else if (privileged_pid < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");
	}

	/* Dropping the root privileges */
	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, cleanup,
			"getpwnam failed for user id %s", nobody_uid);

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	/* this pid is not used by the OS */
	free_pid = tst_get_unused_pid(cleanup);
}

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
			/* Avoid calling glibc wrapper function, as it may
			 * try to read/validate data in cpu mask. This test
			 * is passing invalid pointer on purpose. */
			TEST(ltp_syscall(__NR_sched_setaffinity,
				*(test_cases[i].pid),
				*(test_cases[i].mask_size),
				*(test_cases[i].mask)));

			if (TEST_RETURN != -1)
				tst_resm(TFAIL,
					"sched_setaffinity() unexpectedly succeded");

			if (TEST_ERRNO == test_cases[i].exp_errno) {
				tst_resm(TPASS, "expected failure with '%s'",
					strerror(test_cases[i].exp_errno));
			} else {
				tst_resm(TFAIL,
					"call returned '%s', expected - '%s'",
					strerror(TEST_ERRNO),
					strerror(test_cases[i].exp_errno));
			}
		}
	}

	cleanup();
	tst_exit();
}
