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
#include "usctest.h"

char *TCID = "stream05";
int TST_TOTAL = 1;
extern int Tst_count;
int     local_flag;

#define PASSED 1
#define FAILED 0

char progname[] = "stream05()" ;
char tempfile[40]="";

/*--------------------------------------------------------------------*/
int main(int ac, char *av[])
{
	FILE *stream;
	char buf[10];
	int nr,fd;

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
	tst_tmpdir();
        local_flag = PASSED;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
                local_flag = PASSED;

		sprintf(tempfile, "stream05.%d", getpid());
	/*--------------------------------------------------------------------*/
	//block0:	
		if((stream=fopen(tempfile,"a+")) == NULL) {
			tst_resm(TFAIL,"\tfopen a+ failed\n");
			tst_exit();
		}
		fprintf(stream,"a");
		fclose(stream);

		if((stream=fopen(tempfile,"r+")) == NULL) {
			tst_resm(TFAIL,"\tfopen r+ failed\n");
			tst_exit();
		}

		/* check that ferror returns zero */
		if(ferror(stream) != 0) {
			tst_resm(TFAIL, "\tferror did not return zero\n");
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block0.\n");
		} else {
			tst_resm(TFAIL, "Test failed in block0.\n");
		}

		local_flag = PASSED;

	/*--------------------------------------------------------------------*/
	//block1:	

		/* check that fileno returns valid file descriptor */
		fd=fileno(stream);
		if((nr=read(fd,buf,1)) < 0) {
			tst_resm(TFAIL, "\tread failed\n");
			tst_exit();
		}
		if(nr != 1) {
			tst_resm(TFAIL,"read did not read right number");
			local_flag = FAILED;
		}
		if(buf[0] != 'a') {
			tst_resm(TFAIL, "\tread returned bad values\n");
			local_flag = FAILED;
		}
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block1.\n");
		} else {
			tst_resm(TFAIL, "Test failed in block1.\n");
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
	//block2:	

		/* read to EOF and ensure feof returns non-zero */
		fclose(stream);

		if((stream=fopen(tempfile,"r+")) == NULL) {
			tst_resm(TFAIL,"\tfopen r+ failed\n");
			tst_exit();
		}
		if(feof(stream) != 0) {
			tst_resm(TFAIL, "\tfeof returned non-zero when it should not\n");
			local_flag = FAILED;
		}
		fread(buf,1,2,stream);	/* read to EOF */
		if(feof(stream) == 0) {
			tst_resm(TFAIL, "\tfeof returned zero when it should not\n");
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block2.\n");
		} else {
			tst_resm(TFAIL, "Test failed in block2.\n");
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
	//block3:	
		/* ensure clearerr works */
		clearerr(stream);
		if(feof(stream) != 0) {
			tst_resm(TFAIL, "\tclearerr failed\n");
			local_flag = FAILED;
		}
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block3.\n");
		} else {
			tst_resm(TFAIL, "Test failed in block3.\n");
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
	//block4:	

		/* test fopen "b" flags -- should be allowed but ignored */
		(void) fclose(stream);

		if ((stream = fopen(tempfile, "rb")) == NULL) {
			tst_resm(TFAIL, "\tfopen rb failed\n");
			tst_exit();
		}
		(void) fclose(stream);

		if ((stream = fopen(tempfile, "wb")) == NULL) {
			tst_resm(TFAIL, "\tfopen wb failed\n");
			tst_exit();
		}
		(void) fclose(stream);

		if ((stream = fopen(tempfile, "ab")) == NULL) {
			tst_resm(TFAIL, "\tfopen ab failed\n");
			tst_exit();
		}
		(void) fclose(stream);

		if ((stream = fopen(tempfile, "rb+")) == NULL) {
			tst_resm(TFAIL, "\tfopen rb+ failed\n");
			tst_exit();
		}
		(void) fclose(stream);

		if ((stream = fopen(tempfile, "wb+")) == NULL) {
			tst_resm(TFAIL, "\tfopen wb+ failed\n");
			tst_exit();
		}
		(void) fclose(stream);

		if ((stream = fopen(tempfile, "ab+")) == NULL) {
			tst_resm(TFAIL, "\tfopen ab+ failed\n");
			tst_exit();
		}
		(void) fclose(stream);

		tst_resm(TPASS, "Test passed in block4.\n");
	/*--------------------------------------------------------------------*/
		unlink(tempfile);
	} /* end for */
	tst_rmdir();
	tst_exit();
	/* NOTREACHED */
	return(0);
}
