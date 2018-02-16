/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
* Description:
* Write the current process in argv[1]
* and returns:
*	-> 2 if fork fails with EAGAIN,
*	-> 1 if there is another problem
*	-> 0 if everything worked
*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
	FILE *f;
	int newpid;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s /cgroup/.../tasks\n", argv[0]);
		return 1;
	}

	f = fopen(argv[1], "a");
	if (!f) {
		perror("fopen failed");
		return 1;
	}

	fprintf(f, "%i\n", getpid());
	fclose(f);

	newpid = fork();
	if (newpid == -1 && errno == EAGAIN)
		return 2;
	if (newpid == -1) {
		perror("fork() failed");
		return 1;
	}
	if (newpid == 0)
		pause();
	return 0;
}
