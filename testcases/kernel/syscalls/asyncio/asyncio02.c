/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: asyncio02.c,v 1.6 2009/08/28 11:17:39 vapier Exp $ */
/************************************************************
 * OS Test - Silicon Graphics, Inc.
 * Mendota Heights, Minnesota
 *
 * TEST IDENTIFIER:  aiotcs02:  write/close flushes data to the file
 *
 * PARENT DOCUMENT:  aiotds01:  kernel i/o
 *
 * AUTHOR: Barrie Kletscher
 *
 * CO-PILOT: Dave Baumgartner
 *
 * TEST ITEMS:
 * 	for each open flags set used:
 * 	1. Multiple writes to a file work as specified for
 * 	   more than BUFSIZ bytes.
 * 	2. Multiple writes to a file work as specified for
 * 	   BUFSIZ bytes.
 * 	3. Multiple writes to a file work as specified for
 * 	   lower than BUFSIZ bytes.
 *
 * INPUT SPECIFICATIONS:
 * 	Standard parse_opts supported options.
 *$
 * OUTPUT SPECIFICATIONS
 * 	Standard tst_res output format
 *
 * ENVIRONMENTAL NEEDS:
 * 	This program uses the environment variable TMPDIR for the location
 * 	of the temporary directory.
 *
 *
 * SPECIAL PROCEDURAL REQUIREMENTS:
 * 	The program must be linked with tst_*.o and parse_opts.o.
 *
 * INTERCASE DEPENDENCIES:
 * 	NONE.
 *
 * DETAILED DESCRIPTION:
 * 	Attempt to get some memory to work with.
 * 	Call testrun writing (BUFSIZ + 1) bytes
 * 	Call testrun writing BUFSIZ bytes
 * 	Repeated call to testrun() with decreasing write sizes
 * 		less than BUFSIZ
 * 	End
 *
 * 	Start testrun()
 * 	Attempt to open a temporary file.
 * 	Write the memory to the file.
 * 	Attempt to close the file which also flushes the buffers.
 * 	Now check to see if the number of bytes written is the
 * 		same as the number of bytes in the file.
 * 	Cleanup
 *
 * BUGS:
 * 	NONE.
 *
************************************************************/

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <errno.h>
#include <stdlib.h>
#include "test.h"

#define FLAG O_RDWR | O_CREAT | O_TRUNC	/* Flags used when opening temp tile */
#define MODE  0777		/* Mode to open file with */
#define WRITES 10		/* Number of times buffer is written */
#define DECR 1000		/* Number of bytes decremented between */
					/* Calls to testrun() */
#define OK -1			/* Return value from testrun() */

#define FNAME1	"aio02.1"
#define FNAME2	"aio02.2"
#define FNAME3	"aio02.3"

#define ERR_MSG1 "Bytes in file not equal to bytes written."
#define ERR_MSG2 "Bytes in file (%d) not equal to bytes written (%d)."

char *dp;			/* pointer to area of memory */

void setup();
void cleanup();
int testrun(int flag, int bytes, int ti);

char *TCID = "asyncio02";
int TST_TOTAL = 6;

char *filename;			/* name of the temporary file */

char *Progname;
int Open_flags;

int Flags[] = {
	O_RDWR | O_CREAT | O_TRUNC,
	O_RDWR | O_CREAT | O_TRUNC
};

int Num_flags;

/***********************************************************************
 * MAIN
 ***********************************************************************/
int main(int ac, char **av)
{

	int i;
	int ret_val;
	int eok;		/* everything is ok flag */
	int lc;
	int flag_cnt;

	Num_flags = sizeof(Flags) / sizeof(int);
	TST_TOTAL = 3 * Num_flags;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (flag_cnt = 0; flag_cnt < Num_flags; flag_cnt++) {

			/*
			 * call testrun writing (BUFSIZ + 1) byte chunks
			 */

			filename = FNAME1;
			if (testrun(Flags[flag_cnt], BUFSIZ + 1, 1) != OK) {
				tst_resm(TFAIL, ERR_MSG1);
			} else {
				tst_resm(TPASS,
					 "More than BUFSIZE bytes multiple synchronous writes to a file check out ok");
			}

			/*
			 * call testrun writing BUFSIZ byte chunks
			 */

			filename = FNAME2;
			if (testrun(Flags[flag_cnt], BUFSIZ, 2) != OK) {
				tst_resm(TFAIL, ERR_MSG1);
			} else {
				tst_resm(TPASS,
					 "BUFSIZE bytes multiple synchronous writes to a file checks out ok");
			}

			/*
			 * while the byte chunks are greater than 0
			 *      call testrun() with decreasing chunk sizes
			 */

			filename = FNAME3;
			eok = 1;
			for (i = BUFSIZ - 1; i >= 0; i -= DECR) {
				if ((ret_val =
				     testrun(Flags[flag_cnt], i, 3)) != OK) {
					tst_resm(TFAIL, ERR_MSG2, ret_val,
						 i * WRITES);
				}
			}

			if (eok) {
				tst_resm(TPASS,
					 "Less than BUFSIZE bytes multiple synchronous writes to a file checks out ok");
			}
		}
	}
	cleanup();
	tst_exit();
}				/* end main() */

int testrun(int flag, int bytes, int ti)
{

	int fildes, i, ret;

	struct stat buffer;	/* buffer of memory required for stat command */

	/*
	 *      Attempt to open a temporary file.
	 */

	if ((fildes = open(filename, flag, MODE)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "open(%s) failed", filename);
	}

	/*
	 *      Write the memory to the file.
	 */

	for (i = 0; i < WRITES; i++) {
		TEST(write(fildes, dp, (unsigned)bytes));

		if (TEST_RETURN == -1) {
			tst_brkm(TBROK | TTERRNO, cleanup, "write() failed");
		}
	}			/* end for () */

	/*
	 *      Attempt to close the file which also flushes the buffers.
	 */

	if (close(fildes) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "close() failed");
	}

	ret = OK;

	/*
	 *  Now check to see if the number of bytes written is the
	 *  same as the number of bytes in the file.
	 */

	if (stat(filename, &buffer) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "stat() failed");
	}

	if (buffer.st_size != (off_t) (bytes * WRITES)) {
		ret = (int)buffer.st_size;
	}

	if (unlink(filename) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "unlink(%s) failed",
			 filename);
	}

	return ret;

}				/* end testrun() */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* create a temporary directory and go to it */
	tst_tmpdir();

	/*
	 *  Attempt to get some memory to work with.
	 */

	if ((dp = malloc((unsigned)BUFSIZ + 1)) == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup, "malloc() failed");
	}

}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 ***************************************************************/
void cleanup(void)
{

	tst_rmdir();
}
