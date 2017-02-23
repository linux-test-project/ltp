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

/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * fmtmsg(3C) and addseverity(3C)
 *
 * ALGORITHM
 *      Check basic functionality using various messages and severity levels.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#if !defined(__UCLIBC__)
#include <fmtmsg.h>		/* interface definition */
#endif
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"
#define FAILED 0
#define PASSED 1

char *TCID = "fmtms01";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
int blenter();
int blexit();
void setup();

char ch;
char buf[80];
char *str1 = "LTP:fmtmsg: INFO: LTP fmtmsg() test1 message, NOT an error";
char *str2 = "TO FIX: This is correct output, no action needed  LTP:msg:001";
char *str3 = "LTP:fmtmsg: LTP_TEST: LTP fmtmsg() test2 message, NOT an error";
char *str4 = "TO FIX: This is correct output, no action needed  LTP:msg:002";

void clearbuf(void)
{
	int i;
	for (i = 0; i < 80; i++)
		buf[i] = '\0';
}

#if !defined(__UCLIBC__)

int main(int argc, char *argv[])
{
	int fd, ret_val;
	FILE *fp;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
/*--------------------------------------------------------------*/
	blenter();

	/* Check that system SEV_LEVEL output is correct */

	close(2);		/* redirect stderr to file */
	fd = creat("fmtfile", 0644);
	ret_val = fmtmsg(MM_PRINT | MM_SOFT, "LTP:fmtmsg", MM_INFO,
			 "LTP fmtmsg() test1 message, NOT an error",
			 "This is correct output, no action needed",
			 "LTP:msg:001");
	close(fd);

	if (ret_val != 0) {
		fprintf(temp, "fmtmsg returned %d, expected 0\n\n", ret_val);
		local_flag = FAILED;
	}

	fp = fopen("fmtfile", "r");
	clearbuf();
	fread(buf, sizeof(buf[0]), strlen(str1), fp);
	if (strcmp(str1, buf) != 0) {
		fprintf(temp, "Expected string: %s\n", str1);
		fprintf(temp, "does not match\n");
		fprintf(temp, "received string: %s\n\n", buf);
		local_flag = FAILED;
	}

	/* Read past spaces in output */
	fread(&ch, sizeof(ch), 1, fp);
	while (isspace(ch))
		fread(&ch, sizeof(ch), 1, fp);
	ungetc(ch, fp);

	clearbuf();
	fread(buf, sizeof(buf[0]), strlen(str2), fp);
	fclose(fp);
	if (strcmp(str2, buf) != 0) {
		fprintf(temp, "Expected string: %s\n", str2);
		fprintf(temp, "does not match\n");
		fprintf(temp, "received string: %s\n\n", buf);
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	/* Check that a system defined SEV_LEVEL cannot get redefined */

	ret_val = addseverity(3, "INVALID");
	if (ret_val != MM_NOTOK) {
		fprintf(temp, "addseverity returned %d, expected MM_NOTOK\n",
			ret_val);
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	/* Check that we can define our own */
	/* SEV_LEVEL and output is correct  */

	ret_val = addseverity(5, "LTP_TEST");
	if (ret_val != MM_OK) {
		fprintf(temp, "addseverity returned %d, expected MM_OK\n",
			ret_val);
		local_flag = FAILED;
	}

	close(2);		/* redirect stderr to file */
	fd = creat("fmtfile", 0644);
	ret_val = fmtmsg(MM_PRINT | MM_HARD | MM_OPSYS, "LTP:fmtmsg", 5,
			 "LTP fmtmsg() test2 message, NOT an error",
			 "This is correct output, no action needed",
			 "LTP:msg:002");
	close(fd);

	if (ret_val != 0) {
		fprintf(temp, "fmtmsg returned %d, expected 0\n", ret_val);
		local_flag = FAILED;
	}

	fp = fopen("fmtfile", "r");
	clearbuf();
	fread(buf, sizeof(buf[0]), strlen(str3), fp);
	if (strcmp(str3, buf) != 0) {
		fprintf(temp, "Expected string: %s\n", str3);
		fprintf(temp, "does not match\n");
		fprintf(temp, "received string: %s\n\n", buf);
		local_flag = FAILED;
	}

	/* Read past spaces in output */
	fread(&ch, sizeof(ch), 1, fp);
	while (isspace(ch))
		fread(&ch, sizeof(ch), 1, fp);
	ungetc(ch, fp);

	clearbuf();
	fread(buf, sizeof(buf[0]), strlen(str4), fp);
	if (strcmp(str4, buf) != 0) {
		fprintf(temp, "Expected string: %s\n", str4);
		fprintf(temp, "does not match\n");
		fprintf(temp, "received string: %s\n\n", buf);
		local_flag = FAILED;
	}

	fclose(fp);
	remove("fmtfile");

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	/* Test result of writing to /dev/console */

	ret_val = fmtmsg(MM_CONSOLE | MM_HARD | MM_OPSYS, "LTP:fmtmsg", 5,
			 "LTP fmtmsg() test3 message, NOT an error",
			 "This is correct output, no action needed",
			 "LTP:msg:003");
	if (ret_val != MM_OK) {
		fprintf(temp, "fmtmsg returned %d, expected MM_OK\n", ret_val);
		fprintf(temp, "failed to write to console\n\n");
		local_flag = FAILED;
	}

	blexit();
	anyfail();
	tst_exit();
}

int anyfail(void)
{
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	tst_rmdir();
	tst_exit();
}

void setup(void)
{
	temp = stderr;
	tst_tmpdir();
}

int blenter(void)
{
	local_flag = PASSED;
	return 0;
}

int blexit(void)
{
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	return 0;
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "test is not available on uClibc");
}

#endif /* if !defined(__UCLIBC__) */
