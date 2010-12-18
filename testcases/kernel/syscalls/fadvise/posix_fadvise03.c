/*
 *
 *   Copyright (c) Red Hat Inc., 2007
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	posix_fadvise03.c
 *
 * DESCRIPTION
 *	Check the value that posix_fadvise returns for wrong ADVISE value.
 *
 * USAGE
 *	posix_fadvise03
 *
 * HISTORY
 *	11/2007 Initial version by Masatake YAMATO <yamato@redhat.com>
 *
 * RESTRICTIONS
 *	None
 */

#define _XOPEN_SOURCE 600
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include "test.h"
#include "usctest.h"

#include "linux_syscall_numbers.h"
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 32
#endif

#ifndef __NR_fadvise64
#define __NR_fadvise64 0
#endif

void setup();
void cleanup();

TCID_DEFINE(posix_fadvise03);	/* Test program identifier.    */

char fname[] = "/bin/cat";	/* test executable to open */
int fd = -1;			/* initialized in open */

int expected_error = EINVAL;

int defined_advise[] = {
	POSIX_FADV_NORMAL,
	POSIX_FADV_SEQUENTIAL,
	POSIX_FADV_RANDOM,
	POSIX_FADV_NOREUSE,
	POSIX_FADV_WILLNEED,
	POSIX_FADV_DONTNEED,
};

#define defined_advise_total (sizeof(defined_advise) / sizeof(defined_advise[0]))

#if 0
/* Too many test cases. */
int TST_TOTAL = (INT_MAX - defined_advise_total);
int advise_limit = INT_MAX;
#else
int TST_TOTAL = (32 - defined_advise_total);
int advise_limit = 32;
#endif /* 0 */

/* is_defined_advise:
   Return 1 if advise is in defined_advise.
   Return 0 if not. */
static int is_defined_advise(int advise)
{
	int i;
	for (i = 0; i < defined_advise_total; i++) {
		if (defined_advise[i] == advise)
			return 1;
	}

	return 0;
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int advise;

	/* Check this system has fadvise64 system which is used
	   in posix_fadvise. */
	if ((_FILE_OFFSET_BITS != 64) && (__NR_fadvise64 == 0)) {
		tst_resm(TWARN,
			 "This test can only run on kernels that implements ");
		tst_resm(TWARN, "fadvise64 which is used from posix_fadvise");
		exit(0);
	}

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given on the command line
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* loop through the test cases */
		for (advise = 0; advise < advise_limit; advise++) {

			/* Don't use defiend advise as an argument. */
			if (is_defined_advise(advise)) {
				continue;
			}

			TEST(posix_fadvise(fd, 0, 0, advise));

			if (TEST_RETURN == 0) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			/* Man page says:
			   "On error, an error number is returned." */
			if (TEST_RETURN == expected_error) {
				tst_resm(TPASS,
					 "expected failure - "
					 "returned value = %ld, advise = %d : %s",
					 TEST_RETURN,
					 advise, strerror(TEST_RETURN));
			} else {
				tst_resm(TFAIL,
					 "unexpected return value - %ld : %s, advise %d - "
					 "expected %d",
					 TEST_RETURN,
					 strerror(TEST_RETURN),
					 advise, expected_error);
			}
		}
	}

	/*
	 * cleanup and exit
	 */
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		tst_brkm(TBROK, cleanup,
			 "Unable to open a file(\"%s\") for test: %s\n",
			 fname, strerror(errno));
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	if (fd != -1) {
		close(fd);
	}

}