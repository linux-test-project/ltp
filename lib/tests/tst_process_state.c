/*
 * Copyright (C) 2012 Cyril Hrubis <chrubis@suse.cz>
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

char *TCID = "tst_checkpoint_child";
int TST_TOTAL = 1;

void handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	if (write(2, "Child in sighandler\n", 20)) {
		/* silence build time warnings */
	}
}

int main(void)
{
	int pid;
	volatile int i;

	pid = fork();

	switch (pid) {
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "Fork failed");
		break;
	case 0:
		fprintf(stderr, "Child starts\n");

		signal(SIGALRM, handler);

		for (i = 0; i < 100000000; i++);

		fprintf(stderr, "Child about to sleep\n");

		pause();

		fprintf(stderr, "Child woken up\n");

		return 0;
		break;
	default:
		/* Wait for child to sleep */
		fprintf(stderr, "Parent waits for child to fall asleep\n");
		TST_PROCESS_STATE_WAIT(NULL, pid, 'S');
		fprintf(stderr, "Child sleeping, wake it\n");
		kill(pid, SIGALRM);
		break;
	}

	wait(NULL);
	return 0;
}
