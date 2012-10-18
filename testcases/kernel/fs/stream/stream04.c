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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Ported from SPIE, section2/iosuite/stream4.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < fwrite() fread()
>WHAT:  < 1) Ensure fwrite appends data to stream.
	< 2) Ensure fread and fwrite return values are valid.
>HOW:   < 1) Open a file, write to it, and then check it.
	< 2) Fwrite a know quanity, check return value.
	<    Fread a know quanity, check return value.
>BUGS:  <
======================================================================*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

char *TCID = "stream04";
int TST_TOTAL = 1;
int     local_flag;

#define PASSED 1
#define FAILED 0

char progname[] = "stream04()" ;
char tempfile1[40]="";
long ftell();

/* XXX: add setup and cleanup */

/*--------------------------------------------------------------------*/
int main(int ac, char *av[])
{
	FILE *stream;
	char *junk="abcdefghijklmnopqrstuvwxyz";
	char *inbuf;
	int ret;

	int lc;		 /* loop counter */
	char *msg;	      /* message returned from parse_opts */

	 /*
	  * parse standard options
	  */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	tst_tmpdir();
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		local_flag = PASSED;

		sprintf(tempfile1, "stream04.%d", getpid());
	/*--------------------------------------------------------------------*/
	//block0:
		if ((stream=fopen(tempfile1,"a+")) == NULL) {
			tst_resm(TFAIL|TERRNO,"fopen(%s) a+ failed", tempfile1);
			tst_rmdir();
			tst_exit();
		}
		/* write something and check */
		if ((ret=fwrite(junk,sizeof(*junk),strlen(junk),stream)) == 0) {
			tst_resm(TFAIL,"fwrite failed: %s", strerror(errno));
			tst_rmdir();
			tst_exit();
		}

		if ((size_t)ret != strlen(junk)) {
			tst_resm(TFAIL,"strlen(junk) = %zi != return value from fwrite = %zi", strlen(junk), ret);
			local_flag = FAILED;
		}

		fclose(stream);
		if ((stream=fopen(tempfile1,"r+")) == NULL) {
			tst_resm(TFAIL,"fopen(%s) r+ failed: %s", tempfile1, strerror(errno));
			tst_rmdir();
			tst_exit();
		}
		if ((inbuf=(char *)malloc(strlen(junk))) == 0) {
			tst_resm(TBROK, "test failed because of malloc: %s", strerror(errno));
			tst_rmdir();
			tst_exit();
		}
		if ((ret=fread(inbuf,sizeof(*junk),strlen(junk),stream)) == 0) {
			tst_resm(TFAIL,"fread failed: %s", strerror(errno));
			tst_rmdir();
			tst_exit();
		}
		if ((size_t)ret != strlen(junk)) {
			tst_resm(TFAIL,"strlen(junk) = %zi != return value from fread = %zi", strlen(junk), ret);
			local_flag = FAILED;
		}
		fclose(stream);
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed.");
		} else {
			tst_resm(TFAIL, "Test failed.");
		}
	/*--------------------------------------------------------------------*/
		unlink(tempfile1);
	} /* end for */
	tst_rmdir();
	tst_exit();
}
