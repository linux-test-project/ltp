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
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static void sig_handler(int signo)
{
	_exit(0);
}

int main(int argc, char **argv)
{
	FILE *f;
	struct sigaction sa;

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

	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sig_handler;

	if (sigaction(SIGVTALRM, &sa, NULL)) {
		perror("sigaction failed");
		return 1;
	}

	struct itimerval it = {.it_value = {.tv_sec = 0, .tv_usec = 10000}};

	setitimer(ITIMER_VIRTUAL, &it, NULL);
	for (;;);
	return 0;
}
