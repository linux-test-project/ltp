/*
 * Copyright (c) Itsuro ODA 2004 all rights reserved
 *
 * DTT helper application: issue alarm system-call point.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <err.h>

#define LOOP 100

static void catch(int i)
{
	if (signal(SIGALRM, catch) == SIG_ERR) {
		err(1, NULL); /* exit */
	}

	return;
}

int main(int argc, char *argv[])
{
	int i;

	if (signal(SIGALRM, catch) == SIG_ERR) {
		err(1, NULL); /* exit */
	}

	for (i = 0; i < LOOP; i++) {
		printf("alarm called. (%d)\n", i);
		(void)alarm(1);
		(void)pause();
	}

	return 0;
}
