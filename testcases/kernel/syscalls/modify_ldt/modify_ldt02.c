/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 * NAME
 *	modify_ldt02.c
 *
 * DESCRIPTION
 *	Testcase to check the error conditions for modify_ldt(2)
 *
 * ALGORITHM
 *	block1:
 *		Create a segment at entry 0 and a valid base address.
 *		Read the contents of the segment thru' fs register.
 *		Validate the data.
 *		Write an invalid base address into entry 0.
 *		Read the contents of entry 0 in the child process.
 *		Verify that a SIGSEGV is incurred.
 *
 * USAGE
 *	modify_ldt02
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include "config.h"
#include "test.h"

TCID_DEFINE(modify_ldt02);
int TST_TOTAL = 1;

#if defined(__i386__) && defined(HAVE_MODIFY_LDT)

#ifdef HAVE_ASM_LDT_H
#include <asm/ldt.h>
#endif
extern int modify_ldt(int, void *, unsigned long);

#include <asm/unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

/* Newer ldt.h files use user_desc, instead of modify_ldt_ldt_s */
#ifdef HAVE_STRUCT_USER_DESC
typedef struct user_desc modify_ldt_s;
#elif  HAVE_STRUCT_MODIFY_LDT_LDT_S
typedef struct modify_ldt_ldt_s modify_ldt_s;
#else
typedef struct modify_ldt_ldt_t {
	unsigned int entry_number;
	unsigned long int base_addr;
	unsigned int limit;
	unsigned int seg_32bit:1;
	unsigned int contents:2;
	unsigned int read_exec_only:1;
	unsigned int limit_in_pages:1;
	unsigned int seg_not_present:1;
	unsigned int useable:1;
	unsigned int empty:25;
} modify_ldt_s;
#endif

int create_segment(void *, size_t);
int read_segment(unsigned int);
void cleanup(void);
void setup(void);

#define FAILED 1

int main(int ac, char **av)
{
	int lc;

	int val, pid, status;

	int flag;
	int seg[4];

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

//block1:
		tst_resm(TINFO, "Enter block 1");
		flag = 0;

		seg[0] = 12345;
		if (create_segment(seg, sizeof(seg)) == -1) {
			tst_brkm(TINFO, cleanup, "Creation of segment failed");
		}

		val = read_segment(0);

		if (val != seg[0]) {
			tst_resm(TFAIL, "Invalid value read %d, expected %d",
				 val, seg[0]);
			flag = FAILED;
		}

		if (flag) {
			tst_resm(TINFO, "block 1 FAILED");
		} else {
			tst_resm(TINFO, "block 1 PASSED");
		}

		tst_resm(TINFO, "Exit block 1");

//block2:
		tst_resm(TINFO, "Enter block 2");
		flag = 0;

		if (create_segment(0, 10) == -1) {
			tst_brkm(TINFO, cleanup, "Creation of segment failed");
		}

		tst_old_flush();
		if ((pid = FORK_OR_VFORK()) == 0) {
			val = read_segment(0);
			exit(1);
		}

		(void)waitpid(pid, &status, 0);

		if (WEXITSTATUS(status) != 0) {
			flag = FAILED;
			tst_resm(TFAIL, "Did not generate SEGV, child returned "
				 "unexpected status");
		}

		if (flag) {
			tst_resm(TINFO, "block 2 FAILED");
		} else {
			tst_resm(TINFO, "block 2 PASSED");
		}
	}
	cleanup();
	tst_exit();

}

int create_segment(void *seg, size_t size)
{
	modify_ldt_s entry;

	entry.entry_number = 0;
	entry.base_addr = (unsigned long)seg;
	entry.limit = size;
	entry.seg_32bit = 1;
	entry.contents = 0;
	entry.read_exec_only = 0;
	entry.limit_in_pages = 0;
	entry.seg_not_present = 0;

	return modify_ldt(1, &entry, sizeof(entry));
}

int read_segment(unsigned int index)
{
	int res;
	__asm__ __volatile__("\n\
			push    $0x0007;\n\
			pop     %%fs;\n\
			movl    %%fs:(%1), %0":"=r"(res)
			     :"r"(index * sizeof(int)));
	return res;
}

void sigsegv_handler(int sig)
{
	tst_resm(TINFO, "received signal: %d", sig);
	exit(0);
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	sigemptyset(&act.sa_mask);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	act.sa_handler = sigsegv_handler;
	(void)sigaction(SIGSEGV, &act, NULL);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup(void)
{

}
#elif HAVE_MODIFY_LDT
int main(void)
{
	tst_brkm(TCONF,
		 NULL,
		 "modify_ldt is available but not tested on the platform than __i386__");
}

#else /* if defined(__i386__) */

int main(void)
{
	tst_resm(TINFO, "modify_ldt02 test only for ix86");
	tst_exit();
}

#endif /* if defined(__i386__) */
