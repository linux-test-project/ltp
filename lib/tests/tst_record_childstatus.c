/*
 * Copyright (C) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 */

#include "test.h"

char *TCID = "tst_record_childstatus";
int TST_TOTAL = 1;

int main(void)
{
	int pid;

	pid = tst_fork();

	switch (pid) {
	case 0:
		tst_resm(TFAIL, "Child reports failure");
		/* Uncomment to get child killed by signal */
		//alarm(1);
		//pause();
		tst_exit();
	case -1:
		tst_brkm(TBROK, NULL, "fork failed");
	default:
		tst_record_childstatus(NULL, pid);
	}

	/* Should return failure from child, i.e. exit status is 1 */
	tst_exit();
}
