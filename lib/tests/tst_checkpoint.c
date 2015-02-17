/*
 * Copyright (C) 2012-2015 Cyril Hrubis <chrubis@suse.cz>
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

#include <sys/wait.h>

#include "test.h"

char *TCID = "tst_checkpoint";
int TST_TOTAL = 1;

int main(void)
{
	int pid;

	tst_tmpdir();

	TST_CHECKPOINT_INIT(tst_rmdir);

	pid = fork();

	switch (pid) {
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "Fork failed");
	break;
	case 0:
		fprintf(stderr, "Child: checkpoint signaling\n");
		TST_SAFE_CHECKPOINT_WAKE(NULL, 0);
		exit(0);
	break;
	default:
		TST_SAFE_CHECKPOINT_WAIT(tst_rmdir, 0);
		fprintf(stderr, "Parent: checkpoint reached\n");
	break;
	}

	wait(NULL);
	tst_rmdir();
	return 0;
}
