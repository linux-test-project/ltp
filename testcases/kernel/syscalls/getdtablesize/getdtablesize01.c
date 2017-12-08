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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "getdtablesize01";
int TST_TOTAL = 1;

int main(void)
{
	int table_size, fd = 0, count = 0;
	int max_val_opfiles;
	struct rlimit rlp;

	setup();
	table_size = getdtablesize();
	getrlimit(RLIMIT_NOFILE, &rlp);
	max_val_opfiles = (rlim_t) rlp.rlim_cur;

	tst_resm(TINFO,
		 "Maximum number of files a process can have opened is %d",
		 table_size);
	tst_resm(TINFO,
		 "Checking with the value returned by getrlimit...RLIMIT_NOFILE");

	if (table_size == max_val_opfiles)
		tst_resm(TPASS, "got correct dtablesize, value is %d",
			 max_val_opfiles);
	else {
		tst_resm(TFAIL, "got incorrect table size, value is %d",
			 max_val_opfiles);
		cleanup();
	}

	tst_resm(TINFO,
		 "Checking Max num of files that can be opened by a process.Should be: RLIMIT_NOFILE - 1");
	for (;;) {
		fd = open("/etc/hosts", O_RDONLY);

		if (fd == -1)
			break;
		count = fd;

#ifdef DEBUG
		printf("Opened file num %d\n", fd);
#endif
	}

//Now the max files opened should be RLIMIT_NOFILE - 1 , why ? read getdtablesize man page

	if (count > 0)
		close(count);
	if (count == (max_val_opfiles - 1))
		tst_resm(TPASS, "%d = %d", count, (max_val_opfiles - 1));
	else if (fd < 0 && errno == ENFILE)
		tst_brkm(TCONF, cleanup, "Reached maximum number of open files for the system");
	else
		tst_resm(TFAIL, "%d != %d", count, (max_val_opfiles - 1));

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
