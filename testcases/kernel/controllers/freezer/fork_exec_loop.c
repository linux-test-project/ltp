/*
 * Copyright (c) International Business Machines  Corp., 2008
 * Author: Cedric Le Goater <clg@fr.ibm.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int count = argc > 1 ? atoi(argv[1]) : 20;

	while (1) {
		int i, s;

		for (i = 0; i < count; i++)
			if (fork() == 0)
				execlp("/bin/true", "true", NULL);

		for (i = 0; i < count; i++)
			wait(&s);
	}
}
