/*
 * Copyright (c) International Business Machines  Corp., 2005
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : getdtablesize01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Basic tests for getdtablesize01(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Prashant P Yendigeri
 *                        <prashant.yendigeri@wipro.com>
 *                        Robbie Williamson
 *                        <robbiew@us.ibm.com>
 *
 *    DESCRIPTION
 *      This is a Phase I test for the getdtablesize01(2) system call.
 *      It is intended to provide a limited exposure of the system call.
 *
 **********************************************************/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>
#include "test.h"
#include "usctest.h"

extern void setup();
extern void cleanup();

char *TCID="getdtablesize01";     /* Test program identifier.    */
int TST_TOTAL=1;          	  /* Total number of test cases. */
extern int Tst_count;      	  /* Test Case counter for tst_* routines */

main()
{
 int table_size,loop,fd,count = 0,cnt = 0;

 setup();
 table_size = getdtablesize();

 tst_resm(TINFO,"Maximum number of files a process can have opened is %d",table_size);
 tst_resm(TINFO,"Checking with the value set in fs.h....INR_OPEN");

 if (table_size == INR_OPEN)
 tst_resm(TPASS,"got correct dtablesize, value is %d",INR_OPEN);
 else
 {
   tst_resm(TFAIL,"got incorrect table size, value is %d",INR_OPEN);
   cleanup();
 }

 tst_resm(TINFO,"Checking Max num of files that can be opened by a process. Should get INR_OPEN-1");
 for(loop=1;loop<=INR_OPEN;loop++)
 {
  fd = open("/etc/hosts",O_RDONLY);
#ifdef DEBUG
  printf("Opened file num %d\n",fd);
#endif
  if( fd == -1)
  break;
  else
  count = fd;
 }

//Now the max files opened should be 1024 - 1 = 1023 , why ? read getdtablesize man page

 if(count == (INR_OPEN - 1) )
 tst_resm(TPASS,"%d = %d",count, (INR_OPEN - 1));
 else
 tst_resm(TFAIL,"%d != %d",count, (INR_OPEN - 1));
 cleanup();
}
/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void
setup()
{
    /* capture signals */
    tst_sig(NOFORK, DEF_HANDLER, cleanup);

    /* Pause if that option was specified */
    TEST_PAUSE;
}       /* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 ***************************************************************/
void
cleanup()
{
    /*
     * print timing stats if that option was specified.
     * print errno log if that option was specified.
     */
    TEST_CLEANUP;

    /* exit with return code appropriate for results */
    tst_exit();
}       /* End cleanup() */


