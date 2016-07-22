/*
 *
 * Copyright (c) International Business Machines  Corp., 2002
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
 * http://www.opengroup.org/onlinepubs/009695399/functions/sysconf.html
 *
 * NAME :
 * sysconf01 :  test for sysconf( get configurable system variables) sys call.
 *
 * USAGE :
 *      sysconf01
 */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#define INVAL_FLAG	-1

/** LTP Port **/
#include "test.h"

char *TCID = "sysconf01";
int TST_TOTAL = 56;

static void _test_sysconf(long name, const char *strname)
{
	long retval;

	/* make sure we reset this as sysconf() will not */
	errno = 0;
	retval = sysconf(name);
	if (retval == -1) {

		/*
		 * The manpage for sysconf(2) specifically states that:
		 * 1. If -1 is returned and errno is EINVAL, then the resource
		 * name doesn't exist.
		 * 2. If errno remains 0, then the limit isn't implemented.
		 * 3. Else, something weird happened with the syscall.
		 */
		switch (errno) {
		case EINVAL:
			tst_resm(TCONF, "Resource doesn't exist: %s", strname);
			break;
		case 0:
			tst_resm(TCONF, "Not supported sysconf resource: %s",
				 strname);
			break;
		default:
			tst_resm(TFAIL | TERRNO, "Unexpected errno value for "
				 "%s", strname);
			break;
		}
	} else
		tst_resm(TPASS, "%s = %li", strname, retval);

}

#define test_sysconf(name) _test_sysconf(name, #name)

int main(void)
{
	/* 1 - 5 */
	test_sysconf(_SC_CLK_TCK);
	test_sysconf(_SC_ARG_MAX);
	test_sysconf(_SC_CHILD_MAX);
	test_sysconf(_SC_OPEN_MAX);
	test_sysconf(_SC_JOB_CONTROL);
	/* 6 - 10 */
	test_sysconf(_SC_SAVED_IDS);
	test_sysconf(_SC_VERSION);
	test_sysconf(_SC_PASS_MAX);
	test_sysconf(_SC_LOGIN_NAME_MAX);
	test_sysconf(_SC_XOPEN_VERSION);
	/* 11 - 15 */
	test_sysconf(_SC_TZNAME_MAX);
	test_sysconf(_SC_STREAM_MAX);
	test_sysconf(_SC_XOPEN_CRYPT);
	test_sysconf(_SC_XOPEN_ENH_I18N);
	test_sysconf(_SC_XOPEN_SHM);
	/* 16 - 20 */
	test_sysconf(_SC_XOPEN_XCU_VERSION);
	test_sysconf(_SC_ATEXIT_MAX);
	test_sysconf(_SC_2_C_BIND);
	test_sysconf(_SC_2_C_DEV);
#ifdef _SC_2_C_VERSION
	test_sysconf(_SC_2_C_VERSION);
#else
	tst_resm(TCONF, "_SC_2_C_VERSION not defined");
#endif
	/* 21 - 25 */
	test_sysconf(_SC_2_CHAR_TERM);
	test_sysconf(_SC_2_FORT_DEV);
	test_sysconf(_SC_2_FORT_RUN);
	test_sysconf(_SC_2_LOCALEDEF);
	test_sysconf(_SC_2_SW_DEV);
	/* 26 - 30 */
	test_sysconf(_SC_2_UPE);
	test_sysconf(_SC_2_VERSION);
	test_sysconf(_SC_BC_BASE_MAX);
	test_sysconf(_SC_BC_DIM_MAX);
	test_sysconf(_SC_BC_SCALE_MAX);
	/* 31 - 35 */
	test_sysconf(_SC_BC_STRING_MAX);
	test_sysconf(_SC_COLL_WEIGHTS_MAX);
	test_sysconf(_SC_EXPR_NEST_MAX);
	test_sysconf(_SC_LINE_MAX);
	test_sysconf(_SC_RE_DUP_MAX);
	/* 36 - 40 */
	test_sysconf(_SC_XOPEN_UNIX);
	test_sysconf(_SC_PAGESIZE);
	test_sysconf(_SC_PHYS_PAGES);
	test_sysconf(_SC_AVPHYS_PAGES);
	test_sysconf(_SC_AIO_MAX);
	/* 41 - 45 */
	test_sysconf(_SC_AIO_PRIO_DELTA_MAX);
	test_sysconf(_SC_SEMAPHORES);
	test_sysconf(_SC_SEM_NSEMS_MAX);
	test_sysconf(_SC_SEM_VALUE_MAX);
	test_sysconf(_SC_MEMORY_PROTECTION);
	/* 46 - 50 */
	test_sysconf(_SC_FSYNC);
	test_sysconf(_SC_MEMORY_PROTECTION);
	test_sysconf(_SC_TIMERS);
	test_sysconf(_SC_TIMER_MAX);
	test_sysconf(_SC_MAPPED_FILES);
	/* 51 - 55 */
	test_sysconf(_SC_THREAD_PRIORITY_SCHEDULING);
	test_sysconf(_SC_XOPEN_LEGACY);
	test_sysconf(_SC_MEMLOCK);
	test_sysconf(_SC_XBS5_ILP32_OFF32);
	test_sysconf(_SC_XBS5_ILP32_OFFBIG);

	/* 56 */
	{
		int retval, actual;
		errno = 0;
		retval = sysconf(INVAL_FLAG);
		actual = errno;
		if (retval != -1) {
			tst_resm(TFAIL,
				 "sysconf succeeded for invalid flag (%i), "
				 " retval=%d errno=%d: %s",
				 INVAL_FLAG, retval, actual, strerror(actual));
		} else if (actual != EINVAL) {
			tst_resm(TFAIL,
				 "sysconf correctly failed, but expected "
				 "errno (%i) != actual (%i)", EINVAL, actual);
		} else
			tst_resm(TPASS, "The invalid sysconf key was trapped "
				 "appropriately");
	}

	tst_exit();
}
