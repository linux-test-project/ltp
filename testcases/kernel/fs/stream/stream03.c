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

/* ported from SPIE, section2/filesuite/stream3.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < fseek() ftell()
>WHAT:  < 1) Ensure ftell reports the correct current byte offset.
>HOW:   < 1) Open a file, write to it, reposition the file pointer and
	     check it.
>BUGS:  <
======================================================================*/
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include "test.h"

char *TCID = "stream03";
int TST_TOTAL = 1;
int local_flag;

#define PASSED 1
#define FAILED 0

char progname[] = "stream03()";
char tempfile1[40] = "";

int main(int ac, char *av[])
{
	FILE *stream;
	char buf[30];
	char *junk = "abcdefghijklmnopqrstuvwxyz";
	long pos;
	off_t opos;
	int lc;

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	local_flag = PASSED;
	tst_tmpdir();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		sprintf(tempfile1, "stream03.%d", getpid());
	/*--------------------------------------------------------------------*/
		//block0:

		if ((stream = fopen(tempfile1, "a+")) == NULL) {
			tst_brkm(TBROK, NULL, "fopen(%s) a+ failed: %s",
				 tempfile1,
				 strerror(errno));
		}

		/* make sure offset of zero at start */
		pos = ftell(stream);

		if (pos != 0) {
			tst_resm(TFAIL, "file pointer descrepancy 1");
			local_flag = FAILED;
		}

		/* write something and check */
		if (fwrite(junk, sizeof(*junk), strlen(junk), stream) == 0) {
			tst_brkm(TFAIL, NULL, "fwrite failed: %s",
				 strerror(errno));
		}

		pos = ftell(stream);

		if (pos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 2 (pos=%li)",
				 strlen(junk), pos);
			local_flag = FAILED;
		}

		/* rewind and check */
		rewind(stream);
		pos = ftell(stream);

		if (pos != 0) {
			tst_resm(TFAIL,
				 "file pointer descrepancy 3 (pos=%li, wanted pos=0)",
				 pos);
			local_flag = FAILED;
		}

		/* seek from current position and then check */
		if (fseek(stream, strlen(junk), 1) != 0) {
			tst_brkm(TFAIL, NULL, "fseek failed: %s",
				 strerror(errno));
		}

		pos = ftell(stream);

		if (pos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 4 (pos=%li)",
				 strlen(junk), pos);
			local_flag = FAILED;
		}

		/* seek from end of file and then check */
		if (fseek(stream, 0, 2) != 0) {
			tst_brkm(TFAIL, NULL, "fseek failed: %s",
				 strerror(errno));
		}

		pos = ftell(stream);

		if (pos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 5 (pos=%li)",
				 strlen(junk), pos);
			local_flag = FAILED;
		}

		/* rewind with seek and then check */
		if (fseek(stream, 0, 0) != 0) {
			tst_brkm(TFAIL, NULL, "fseek failed: %s",
				 strerror(errno));
		}

		pos = ftell(stream);

		if (pos != 0) {
			tst_resm(TFAIL,
				 "file pointer descrepancy 6 (pos=%li, wanted pos=0)",
				 pos);
			local_flag = FAILED;
		}

		/* read till EOF, do getc and then check ftell */
		while (fgets(buf, sizeof(buf), stream)) ;
		pos = ftell(stream);
		getc(stream);
		pos = ftell(stream);

		if (pos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 7 (pos=%li)",
				 strlen(junk), pos);
			local_flag = FAILED;
		}

		fclose(stream);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block0.");
		} else {
			tst_resm(TFAIL, "Test failed in block0.");
		}

		local_flag = PASSED;

		unlink(tempfile1);
	/*--------------------------------------------------------------------*/
		//block1:
		if ((stream = fopen(tempfile1, "a+")) == NULL) {
			tst_brkm(TFAIL, NULL, "fopen(%s) a+ failed: %s",
				 tempfile1,
				 strerror(errno));
		}

		/* make sure offset of zero at start */
		opos = ftello(stream);

		if (opos != 0) {
			tst_resm(TFAIL,
				 "file pointer descrepancy 1 (opos=%" PRId64
				 ", wanted opos=0)", (int64_t) opos);
			local_flag = FAILED;
		}

		/* write something and check */
		if (fwrite(junk, sizeof(*junk), strlen(junk), stream) == 0) {
			tst_brkm(TFAIL, NULL, "fwrite failed: %s",
				 strerror(errno));
		}

		opos = ftello(stream);

		if (opos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 2 (opos=%"
				 PRId64 ")", strlen(junk), (int64_t) opos);
			local_flag = FAILED;
		}

		/* rewind and check */
		rewind(stream);
		opos = ftello(stream);

		if (opos != 0) {
			tst_resm(TFAIL,
				 "file pointer descrepancy 3 (opos=%" PRId64
				 ", wanted opos=0)", (int64_t) opos);
			local_flag = FAILED;
		}

		/* seek from current position and then check */
		if (fseeko(stream, strlen(junk), 1) != 0) {
			tst_brkm(TFAIL, NULL, "fseeko failed: %s",
				 strerror(errno));
		}

		opos = ftello(stream);

		if (opos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 4 (opos=%"
				 PRId64 ")", strlen(junk), (int64_t) opos);
			local_flag = FAILED;
		}

		/* seek from end of file and then check */
		if (fseeko(stream, 0, 2) != 0) {
			tst_brkm(TFAIL, NULL, "fseeko failed: %s",
				 strerror(errno));
		}

		opos = ftello(stream);

		if (opos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 5 (opos=%"
				 PRId64 ")", strlen(junk), (int64_t) opos);
			local_flag = FAILED;
		}

		/* rewind with seek and then check */
		if (fseeko(stream, 0, 0) != 0) {
			tst_brkm(TFAIL, NULL, "fseeko failed: %s",
				 strerror(errno));
		}

		opos = ftello(stream);

		if (opos != 0) {
			tst_resm(TFAIL,
				 "file pointer descrepancy 6 (opos=%" PRId64
				 ", wanted opos=0)", (int64_t) opos);
			local_flag = FAILED;
		}

		/* read till EOF, do getc and then check ftello */
		while (fgets(buf, sizeof(buf), stream)) ;

		opos = ftello(stream);
		getc(stream);
		opos = ftello(stream);

		if (opos != strlen(junk)) {
			tst_resm(TFAIL,
				 "strlen(junk)=%zi: file pointer descrepancy 7 (opos=%li)",
				 strlen(junk), opos);
			local_flag = FAILED;
		}

		fclose(stream);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block1.");
		} else {
			tst_resm(TFAIL, "Test failed in block1.");
		}

		unlink(tempfile1);
	}

	tst_rmdir();
	tst_exit();
}
