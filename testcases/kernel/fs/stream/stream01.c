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

/* ported from SPIE section2/filesuite/stream1.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < freopen()
>WHAT:  < 1) check that freopen substitutes the named file in place of stream.
>HOW:   < 1) open a stream, write something to it, perform freopen and
	<    write some more. Check that second write to stream went to
	<    the file specified by freopen.
>BUGS:  <
======================================================================*/

#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "stream01";
int TST_TOTAL = 1;
extern int Tst_count;
int     local_flag;

#define PASSED 1
#define FAILED 0


char progname[] = "stream01()" ;
char tempfile1[40]="";
char tempfile2[40]="";

/*--------------------------------------------------------------------*/
int main(int ac, char *av[])
{
	FILE *stream;
	char buf[10];
	int i;
        int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */

         /*
          * parse standard options
          */
        if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
                        tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
                 tst_exit();
                 /*NOTREACHED*/
         }

        local_flag = PASSED;
	tst_tmpdir();
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		sprintf(tempfile1, "stream011.%d", getpid());
		sprintf(tempfile2, "stream012.%d", getpid());
	/*--------------------------------------------------------------------*/
	//block0:
		if((stream=fopen(tempfile1,"a+")) == NULL) {
			tst_resm(TFAIL,"fopen(%s) a+ failed: %s", tempfile1, strerror(errno));
			tst_exit();
		}
		fwrite("a",1,1,stream);
		if((stream=freopen(tempfile2,"a+",stream)) == NULL) {
			tst_resm(TFAIL,"freopen(%s) a+ failed: %s", tempfile2, strerror(errno));
			tst_exit();
		}
		fwrite("a",1,1,stream);
		fclose(stream);

		/* now check that a single "a" is in each file */
		if((stream=fopen(tempfile1,"r")) == NULL) {
			tst_resm(TFAIL,"fopen(%s) r failed: %s", tempfile1, strerror(errno));
			tst_exit();
		}
		else {
			for(i=0; i<10; i++) buf[i]=0;
			fread(buf,1,1,stream);
			if((buf[0] != 'a') || (buf[1] != 0)) {
				tst_resm(TFAIL,"bad contents in %s", tempfile1);
				local_flag = FAILED;
			}
			fclose(stream);
		}
		if((stream=fopen(tempfile2,"r")) == NULL) {
			tst_resm(TFAIL,"fopen(%s) r failed: %s", tempfile2, strerror(errno));
			tst_exit();
		}
		else {
			for(i=0; i<10; i++) buf[i]=0;
			fread(buf,1,1,stream);
			if((buf[0] != 'a') || (buf[1] != 0)) {
				tst_resm(TFAIL,"bad contents in %s", tempfile2);
				local_flag = FAILED;
			}
			fclose(stream);
		}
		 if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed.");
		 } else {
			tst_resm(TFAIL, "Test failed.");
		 }

		 local_flag = PASSED;

	/*--------------------------------------------------------------------*/
		unlink(tempfile1);
		unlink(tempfile2);

	} /* end for */
	tst_rmdir();
	tst_exit();
	return 0;
}
