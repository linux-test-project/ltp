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

/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	abort
 *
 * CALLS
 *	abort(3)
 *
 * ALGORITHM
 *	Fork child.  Have child abort, check return status.
 *
 * RESTRICTIONS
 *      The ulimit for core file size must be greater than 0.
 *
 * CHANGE LOG:
 * Nov 11 2002: Ported to LTP Suite by Ananda
 *
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"
#define ITER	3
#define FAILED 0
#define PASSED 1

//char progname[]= "abort1()";
char *TCID = "abort01";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
extern int Tst_count;

int anyfail();
int instress();
void setup();
void terror();
void fail_exit();
void ok_exit();
int forkfail();
void do_child();

/*************/

/*--------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	register int i;
	int status, count, child, kidpid;
	int core, sig, ex;
	char *msg;

	if ((msg =
	     parse_opts(argc, argv, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();		/* temp file is now open */
/*--------------------------------------------------------------*/

	for (i = 0; i < ITER; i++) {

		if ((kidpid = FORK_OR_VFORK()) == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "")) {
				terror("self_exec failed (may be OK if under stress)");
				if (instress())
					ok_exit();
				forkfail();
			}
#else
			do_child();
#endif
		}
		if (kidpid < 0) {
			terror("Fork failed (may be OK if under stress)");
			if (instress())
				ok_exit();
			forkfail();
		}
		count = 0;
		while ((child = wait(&status)) > 0) {
			count++;
		}
		if (count != 1) {
			fprintf(temp, "\twrong # children waited on.\n");
			fprintf(temp, "\tgot %d, expected %d\n", count, 1);
			fail_exit();
		}
		/*
		   sig = status & 0177;
		   core = status & 0200;
		   ex = (status & 0xFF00) >> 8;
		 */
		/*****	LTP Port	*****/
		sig = WTERMSIG(status);
#ifdef WCOREDUMP
		core = WCOREDUMP(status);
#endif
		ex = WIFEXITED(status);
		/**************/
		if (!core) {
			fprintf(temp, "\tChild did not return core bit set!\n");
			fprintf(temp, "\t  iteration %d, exit stat = 0x%x\n",
				i, status);
			fprintf(temp, "\tCore = %d, sig = %d, ex = %d\n",
				core, sig, ex);
			local_flag = FAILED;
		}
		if (sig != SIGIOT) {
			fprintf(temp, "\tChild did not exit with SIGIOT (%d)\n",
				SIGIOT);
			fprintf(temp, "\t  iteration %d, exit stat = 0x%x\n",
				i, status);
			fprintf(temp, "\tCore = %d, sig = %d, ex = %d\n",
				core, sig, ex);
			local_flag = FAILED;
		}
		if (local_flag == FAILED)
			break;
	}

/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to anyfail.	*/

	unlink("core");
	anyfail();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
	return 0;
}

/*--------------------------------------------------------------*/

void do_child()
{
	abort();
	fprintf(temp, "\tchild - abort failed.\n");
	exit(0);
}

/******	LTP Port	*****/
int anyfail()
{
	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed") :
				tst_resm(TPASS, "Test passed");
	tst_rmdir();
	tst_exit();
	return 0;
}

void setup()
{
	temp = stderr;
	tst_tmpdir();
}

int instress()
{
	tst_resm(TINFO,
		 "System resources may be too low; fork(), select() etc are likely to fail.");
	return 1;
}

void terror(char *message)
{
	tst_resm(TBROK, "Reason: %s:%s", message, strerror(errno));
}

void fail_exit()
{
	local_flag = FAILED;
	anyfail();

}

void ok_exit()
{
	local_flag = PASSED;
	tst_resm(TINFO, "Test passed");
}

int forkfail()
{
	fprintf(temp, "\t\tFORK FAILED - terminating test.\n");
	tst_exit();
	return 0;
}

/*****************/
