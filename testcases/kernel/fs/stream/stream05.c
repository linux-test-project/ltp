/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* ported from SPIE, section2/filesuite/stream.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < ferror() feof() clearerr() fileno()
>WHAT:  < 1) check that ferror returns zero
	< 2) check fileno returns valid file descriptor
	< 3) check that feof returns zero (nonzero) appropriately
	< 4) check that clearerr resets EOF indicator.
>HOW:   < 1) open a stream and immediately execute ferror
	< 2) use the file des returned from fileno to read a file
	<    written with stream - compare actual vs expected.
	< 3) open stream and ensure feof returns zero, read to end of
	<    file and ensure feof returns non-zero.
	< 4) after 3) above use clearerr and then use feof to ensure
	<    clearerr worked
>BUGS:  <
======================================================================*/

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "test.h"

char *TCID = "stream05";
int TST_TOTAL = 1;
int local_flag;

#define PASSED 1
#define FAILED 0

char progname[] = "stream05()";
char tempfile[40] = "";

/*--------------------------------------------------------------------*/
int main(int ac, char *av[])
{
	FILE *stream;
	char buf[10];
	int nr, fd;

	int lc;

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);
	tst_tmpdir();
	local_flag = PASSED;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		local_flag = PASSED;

		sprintf(tempfile, "stream05.%d", getpid());
	/*--------------------------------------------------------------------*/
		//block0:
		if ((stream = fopen(tempfile, "a+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) a+ failed: %s",
				 tempfile,
				 strerror(errno));
		}
		fprintf(stream, "a");
		fclose(stream);

		if ((stream = fopen(tempfile, "r+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) r+ failed: %s",
				 tempfile,
				 strerror(errno));
		}

		/* check that ferror returns zero */
		if (ferror(stream) != 0) {
			tst_resm(TFAIL, "ferror did not return zero: %s",
				 strerror(errno));
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block0.");
		} else {
			tst_resm(TFAIL, "Test failed in block0.");
		}

		local_flag = PASSED;

	/*--------------------------------------------------------------------*/
		//block1:

		/* check that fileno returns valid file descriptor */
		fd = fileno(stream);
		if ((nr = read(fd, buf, 1)) < 0) {
			tst_brkm(TFAIL, NULL, "read failed: %s",
				 strerror(errno));
		}
		if (nr != 1) {
			tst_resm(TFAIL, "read did not read right number");
			local_flag = FAILED;
		}
		if (buf[0] != 'a') {
			tst_resm(TFAIL, "read returned bad values");
			local_flag = FAILED;
		}
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block1.");
		} else {
			tst_resm(TFAIL, "Test failed in block1.");
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
		//block2:

		/* read to EOF and ensure feof returns non-zero */
		fclose(stream);

		if ((stream = fopen(tempfile, "r+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) r+ failed: %s",
				 tempfile,
				 strerror(errno));
		}
		if (feof(stream) != 0) {
			tst_resm(TFAIL,
				 "feof returned non-zero when it should not: %s",
				 strerror(errno));
			local_flag = FAILED;
		}
		fread(buf, 1, 2, stream);	/* read to EOF */
		if (feof(stream) == 0) {
			tst_resm(TFAIL,
				 "feof returned zero when it should not: %s",
				 strerror(errno));
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block2.");
		} else {
			tst_resm(TFAIL, "Test failed in block2.");
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
		//block3:
		/* ensure clearerr works */
		clearerr(stream);
		if (feof(stream) != 0) {
			tst_resm(TFAIL, "clearerr failed: %s", strerror(errno));
			local_flag = FAILED;
		}
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block3.");
		} else {
			tst_resm(TFAIL, "Test failed in block3.");
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
		//block4:

		/* test fopen "b" flags -- should be allowed but ignored */
		(void)fclose(stream);

		if ((stream = fopen(tempfile, "rb")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) rb failed: %s",
				 tempfile,
				 strerror(errno));
		}
		(void)fclose(stream);

		if ((stream = fopen(tempfile, "wb")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) wb failed: %s",
				 tempfile,
				 strerror(errno));
		}
		(void)fclose(stream);

		if ((stream = fopen(tempfile, "ab")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) ab failed: %s",
				 tempfile,
				 strerror(errno));
		}
		(void)fclose(stream);

		if ((stream = fopen(tempfile, "rb+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) rb+ failed: %s",
				 tempfile,
				 strerror(errno));
		}
		(void)fclose(stream);

		if ((stream = fopen(tempfile, "wb+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) wb+ failed: %s",
				 tempfile,
				 strerror(errno));
		}
		(void)fclose(stream);

		if ((stream = fopen(tempfile, "ab+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) ab+ failed: %s",
				 tempfile,
				 strerror(errno));
		}
		(void)fclose(stream);

		tst_resm(TPASS, "Test passed in block4.");
	/*--------------------------------------------------------------------*/
		unlink(tempfile);
	}			/* end for */
	tst_rmdir();
	tst_exit();
}
