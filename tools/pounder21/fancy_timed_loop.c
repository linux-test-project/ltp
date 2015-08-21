/* Repeatedly run a program with a given uid, gid and termination signal. */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"

static int res = 0;
static char *progname;
static pid_t test_pgrp;
static FILE *out;

static int the_signal = SIGTERM;

static void int_func(int signum)
{
	pounder_fprintf(out,
			"%s: Killed by interrupt.  Last exit code = %d.\n",
			progname, res);
	kill(-test_pgrp, the_signal);
	exit(res);
}

static void alarm_func(int signum)
{
	pounder_fprintf(out, "%s: Killed by timer.  Last exit code = %d.\n",
			progname, res);
	kill(-test_pgrp, the_signal);
	exit(res);
}

int main(int argc, char *argv[])
{
	int secs, stat;
	pid_t pid;
	unsigned int revs = 0;
	struct sigaction zig;
	uid_t uid;
	gid_t gid;
	int use_max_failures = 0;
	int max_failures = 0;
	int fail_counter = 1;

	if (argc < 5) {
		printf
		    ("Usage: %s [-m max_failures] time_in_sec uid gid signal command [args]\n",
		     argv[0]);
		exit(1);
	}
	//by default, set max_failures to whatever the env variable $MAX_FAILURES is
	char *max_failures_env = getenv("MAX_FAILURES");
	max_failures = atoi(max_failures_env);

	//if the -m option is used when calling fancy_timed_loop, override max_failures
	//specified by $MAX_FAILURES with the given argument instead
	if (argc > 6 && strcmp(argv[1], "-m") == 0) {
		if ((max_failures = atoi(argv[2])) >= 0) {
			use_max_failures = 1;
		} else {
			printf
			    ("Usage: %s [-m max_failures] time_in_sec uid gid signal command [args]\n",
			     argv[0]);
			printf
			    ("max_failures should be a nonnegative integer\n");
			exit(1);
		}
	}

	out = stdout;

	if (use_max_failures) {
		progname = rindex(argv[7], '/');
		if (progname == NULL) {
			progname = argv[7];
		} else {
			progname++;
		}
	} else {
		progname = rindex(argv[5], '/');
		if (progname == NULL) {
			progname = argv[5];
		} else {
			progname++;
		}
	}

	/* Set up signals */
	memset(&zig, 0x00, sizeof(zig));
	zig.sa_handler = alarm_func;
	sigaction(SIGALRM, &zig, NULL);
	zig.sa_handler = int_func;
	sigaction(SIGINT, &zig, NULL);
	sigaction(SIGTERM, &zig, NULL);

	/* set up process groups so that we can kill the
	 * loop test and descendants easily */

	if (use_max_failures) {
		secs = atoi(argv[3]);
		alarm(secs);

		the_signal = atoi(argv[6]);
		uid = atoi(argv[4]);
		gid = atoi(argv[5]);
	} else {
		secs = atoi(argv[1]);
		alarm(secs);

		the_signal = atoi(argv[4]);
		uid = atoi(argv[2]);
		gid = atoi(argv[3]);
	}

	pounder_fprintf(out, "%s: uid = %d, gid = %d, sig = %d\n",
			progname, uid, gid, the_signal);

	while (1) {
		pounder_fprintf(out, "%s: %s loop #%d.\n", progname,
				start_msg, revs++);
		pid = fork();
		if (pid == 0) {
			// set process group
			if (setpgrp() < 0) {
				perror("setpgid");
			}
			// set group and user id
			if (setregid(gid, gid) != 0) {
				perror("setregid");
				exit(-1);
			}

			if (setreuid(uid, uid) != 0) {
				perror("setreuid");
				exit(-1);
			}
			// run the program
			if (use_max_failures) {
				if (argc > 5) {
					stat = execvp(argv[7], &argv[7]);
				} else {
					stat = execvp(argv[7], &argv[7]);
				}

				perror(argv[7]);
			} else {
				if (argc > 3) {
					stat = execvp(argv[5], &argv[5]);
				} else {
					stat = execvp(argv[5], &argv[5]);
				}

				perror(argv[5]);
			}

			exit(-1);
		}

		/* save the pgrp of the spawned process */
		test_pgrp = pid;

		// wait for it to be done
		if (waitpid(pid, &stat, 0) != pid) {
			perror("waitpid");
			exit(1);
		}
		// interrogate it
		if (WIFSIGNALED(stat)) {
			pounder_fprintf(out, "%s: %s on signal %d.\n",
					progname, fail_msg, WTERMSIG(stat));
			res = 255;
		} else {
			res = WEXITSTATUS(stat);
			if (res == 0) {
				pounder_fprintf(out, "%s: %s.\n", progname,
						pass_msg);
			} else if (res < 0 || res == 255) {
				pounder_fprintf(out,
						"%s: %s with code %d.\n",
						progname, abort_msg, res);
				exit(-1);
				// FIXME: add test to blacklist
			} else {
				pounder_fprintf(out,
						"%s: %s with code %d.\n",
						progname, fail_msg, res);
				if (max_failures > 0) {
					if (++fail_counter > max_failures) {
						exit(-1);
					}
				}
			}
		}
	}
}
