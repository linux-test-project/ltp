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
/* ported from SPIE section2/filesuite/stream2.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < fseek() mknod() fopen()
>WHAT:  < 1)
>HOW:   < 1)
>BUGS:  < 
======================================================================*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

char *TCID = "stream02";
int TST_TOTAL = 1;
extern int Tst_count;
int     local_flag;

#define PASSED 1
#define FAILED 0


char progname[] = "stream02()" ;
char tempfile1[40]="";

/*--------------------------------------------------------------------*/
int main(int ac, char *av[])
{
	FILE *stream;
	int fd;
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

		sprintf(tempfile1, "stream1.%d", getpid());
	/*--------------------------------------------------------------------*/
	//block0:
		if(mknod(tempfile1, (S_IFIFO|0666), 0) != 0) {
			tst_resm(TFAIL,"\tmknod failed in block0\n");
			local_flag = FAILED;
			goto block1;
		}
		if((stream=fopen(tempfile1,"w+")) == NULL) {
			tst_resm(TFAIL,"\tfopen w+ failed for pipe file\n");
			local_flag = FAILED;
		} else {
			fclose(stream);
		}
		if((stream=fopen(tempfile1,"a+")) == NULL) {
			tst_resm(TFAIL,"\tfopen a+ failed\n");
			local_flag = FAILED;
		} else {
			fclose(stream);
			unlink(tempfile1);
		}
		if (local_flag == PASSED) {
		         tst_resm(TPASS, "Test passed in block0.\n");
		} else {
		         tst_resm(TFAIL, "Test failed in block0.\n");
	        }
		local_flag = PASSED;

	/*--------------------------------------------------------------------*/
	block1 : 
		if(( fd = open("/dev/tty",O_WRONLY)) >= 0 )
		{
			close(fd);
			if(( stream = fopen("/dev/tty","w"))==NULL) {
				tst_resm(TFAIL,"\tfopen write  failed for /dev/tty\n");
				local_flag = FAILED;
			} else {
				fclose(stream);
			}
		}
		if (local_flag == PASSED) {
		         tst_resm(TPASS, "Test passed in block1.\n");
		} else {
		         tst_resm(TFAIL, "Test failed in block1.\n");
	        }
			
	/*--------------------------------------------------------------------*/
	} /* end for */
	tst_rmdir();
	tst_exit();
        return(0);
}
