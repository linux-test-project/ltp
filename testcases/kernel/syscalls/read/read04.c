/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	read04.c
 *
 * DESCRIPTION
 *	Testcase to check if read returns the number of bytes read correctly.
 *
 * ALGORITHM
 *	Create a file and write some bytes out to it.
 *	Attempt to read more than written.
 *	Check the return count, and the read buffer. The read buffer should be
 *	same as the write buffer.
 *
 * USAGE:  <for command-line>
 *  read04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"

void cleanup(void);
void setup(void);

char *TCID = "read04";
int TST_TOTAL = 1;

#define TST_SIZE	27	/* could also do strlen(palfa) */
char fname[255];
char palfa[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int fild;

int main(int ac, char **av)
{
	int lc;

	int rfild;
	char prbuf[BUFSIZ];

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup for test */

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;	/* reset tst_count while looping */

		if ((rfild = open(fname, O_RDONLY)) == -1) {
			tst_brkm(TBROK, cleanup, "can't open for reading");
		}
		TEST(read(rfild, prbuf, BUFSIZ));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "call failed unexpectedly");
			continue;
		}

		if (TEST_RETURN != TST_SIZE) {
			tst_resm(TFAIL, "Bad read count - got %ld - "
				 "expected %d", TEST_RETURN, TST_SIZE);
			continue;
		}
		if (memcmp(palfa, prbuf, TST_SIZE) != 0) {
			tst_resm(TFAIL, "read buffer not equal "
				 "to write buffer");
			continue;
		}
		tst_resm(TPASS, "functionality of read() is correct");

		SAFE_CLOSE(cleanup, rfild);
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "tfile_%d", getpid());

	if ((fild = creat(fname, 0777)) == -1) {
		tst_brkm(TBROK, cleanup, "creat(%s, 0777) Failed, errno = %d"
			 " : %s", fname, errno, strerror(errno));
	}
	if (write(fild, palfa, TST_SIZE) != TST_SIZE) {
		tst_brkm(TBROK, cleanup, "can't write to Xread");
	}
	close(fild);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 *	       premature exit.
 */
void cleanup(void)
{

	unlink(fname);
	tst_rmdir();

}
