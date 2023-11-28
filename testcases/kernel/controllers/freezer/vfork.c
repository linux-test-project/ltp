/*
 * Copyright (c) International Business Machines  Corp., 2008
 * Author: Matt Helsley <matthltc@us.ibm.com>
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
 *
 *
 * Usage: $0 <num>
 *
 * vfork <num> times, stopping after each vfork. TODO: Requires an external process
 * to send SIGCONT to goto the next vfork. <num> SIGCONT signals must be
 * received before exiting.
 *
 * We can't do anything but execve or _exit in vfork'd processes
 * so we use ptrace vfork'd processes in order to pause then during each
 * vfork. This places the parent process in "TASK_UNINTERRUPTIBLE" state
 * until vfork returns. This can delay delivery of signals to the parent
 * process, even delay or stop system suspend.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ptrace.h>
#include "test.h"

#define str_expand(s) str(s)
#define str(s) #s

#define debug(s) \
perror("ERROR at " __FILE__ ":" str_expand(__LINE__) ": " s )

char *filename = NULL;
FILE *fp = NULL;
int psync[2];
pid_t child = -1;

int TST_TOTAL = 1;
char *TCID = "vfork";

/* for signal handlers */
void parent_cleanup(void)
{
	close(psync[1]);
	if (fp) {
		fflush(fp);
		if (filename) {
			fclose(fp);
			(void)unlink(filename);
		}
	}
	tst_exit();
}

void kill_child(void)
{

	/* Avoid killing all processes at the current user's level, and the
	 * test app as well =].
	 */
	if (0 < child && kill(child, 0) == 0) {
		/* Shouldn't happen, but I've seen it before... */
		if (ptrace(PTRACE_KILL, child, NULL, NULL) < 0) {
			tst_resm(TBROK | TERRNO,
				 "ptrace(PTRACE_KILL, %d, ..) failed", child);
		}
		(void)waitpid(child, NULL, WNOHANG);	/* Zombie children are bad. */
	}

}

void child_cleanup(void)
{
	close(psync[0]);
	tst_exit();
}

int do_vfork(int count)
{
	pid_t child;

	while (count) {
		child = vfork();
		if (child == 0)
			_exit(0);
		else if (child > 0)
			count--;
		else {
			tst_brkm(TFAIL | TERRNO, NULL, "vfork failed");
		}
	}

	return EXIT_SUCCESS;
}

/* Options */
int num_vforks = 1;
int do_pause = 0;
int do_sleep = 0;
struct timespec sleep_duration;

void sleepy_time(void)
{
	do {
		int rc = nanosleep(&sleep_duration, &sleep_duration);

		if ((rc == -1) && (errno != EINTR))
			continue;
		break;
	} while (1);
}

void usage(void)
{
	tst_resm(TBROK, "usage: %s [-f [FILE]] [-s [NUM]] [-p] [NUM]\n\n"
		 "\t-f FILE\t\tFile to output trace data to.\n"
		 "\t-s NUM\t\tSleep for NUM seconds. [Default: 1 second]\n"
		 "\t\t\t\tSuffixes ms, us, s, m, and h correspond to\n"
		 "\t\t\t\tmilliseconds, microseconds, seconds [Default],\n"
		 "\t\t\t\tminutes, and hours respectively.\n\n"
		 "\t-p\t\tPause.\n\n"
		 "\tNUM\t\tExecute vfork NUM times.\n", TCID);
}

void _parse_opts(int argc, char **argv)
{
	int opt;
	char *units;
	unsigned long long duration = 1U;

	sleep_duration.tv_sec = 0U;
	sleep_duration.tv_nsec = 0U;

	while ((opt = getopt(argc, argv, "f:ps::")) != -1) {
		switch (opt) {
		case 'f':
			if ((fp = fopen(optarg, "w")) != NULL) {
				filename = optarg;
			}
			break;
		case 'p':
			do_pause = 1;
			break;
		case 's':
			if (optarg == NULL) {
				sleep_duration.tv_sec = 1;
				do_sleep = 1;
				break;
			}
			opt = sscanf(optarg, "%Ld%as", &duration, &units);
			if (opt < 1)
				break;

			if ((opt != 2) || !strcmp(units, "s"))
				sleep_duration.tv_sec = duration;
			else if (!strcmp(units, "ms"))
				sleep_duration.tv_nsec = duration * 1000000U;
			else if (!strcmp(units, "us"))
				sleep_duration.tv_nsec = duration * 1000U;
			else if (!strcmp(units, "m"))
				sleep_duration.tv_sec = duration * 60U;
			else if (!strcmp(units, "h"))
				sleep_duration.tv_sec = duration * 3600U;
			else {
				tst_resm(TBROK, "Unrecognized time units: %s",
					 units);
				usage();
			}
			do_sleep = 1;
			break;
		default:
			usage();
		}
	}

	if (optind >= argc)
		return;
	if (!strcmp(argv[optind], "--"))
		return;
	sscanf(argv[optind], "%d", &num_vforks);
}

int trace_grandchild(pid_t gchild)
{
#if HAVE_DECL_PTRACE_GETSIGINFO
	siginfo_t info;

	if (ptrace(PTRACE_GETSIGINFO, gchild, NULL, &info) == -1) {
		debug("ptrace(): ");
		return 0;
	}
	/*dump_siginfo(gchild, &info); */
	if ((info.si_code != 0) || (info.si_signo != SIGSTOP))
		return 0;

	tst_resm(TINFO, "Grandchild spawn's pid=%d", gchild);
	fprintf(fp, "\t%d\n", gchild);
	fflush(fp);
	if (do_pause)
		pause();
	if (do_sleep)
		sleepy_time();
	if (ptrace(PTRACE_DETACH, gchild, NULL, NULL) == -1)
		debug("ptrace(): ");
	return -1;		/* don't wait for gchild */
#else
	return 0;
#endif
}

int do_trace(pid_t child, int num_children)
{
	int my_exit_status = EXIT_SUCCESS;
	int status;
	pid_t process;

	while (num_children > 0) {
		int died = 0;

		/*printf("waiting for %d processes to exit\n", num_children); */
		process = waitpid(-1, &status, WUNTRACED);
		if (process < 1)
			continue;
		/*dump_status(process, status); */
		died = (WIFEXITED(status) || WIFSIGNALED(status));
		if (died)
			num_children--;
		if (process == child)
			my_exit_status = WEXITSTATUS(status);
		if (died || !WIFSTOPPED(status))
			continue;

		if (process == child) {
			/* trace_child(process); */
			if (ptrace(PTRACE_CONT, process, NULL, NULL) == -1)
				debug("ptrace(): ");
		} else
			num_children += trace_grandchild(process);

	}

	return my_exit_status;
}

void send_mutex(int fd)
{
	ssize_t nbytes = 0;

	do {
		nbytes = write(fd, "r", 1);
		if (nbytes == 1)
			break;
		if (nbytes != -1)
			continue;
		if ((errno == EAGAIN) || (errno == EINTR))
			continue;
		else
			exit(EXIT_FAILURE);
		debug("write: ");
	} while (1);
}

void await_mutex(int fd)
{
	char buffer[1];
	ssize_t nbytes = 0;

	do {
		nbytes = read(fd, buffer, sizeof(buffer));
		if (nbytes == 1)
			break;
		if (nbytes != -1)
			continue;
		if ((errno == EAGAIN) || (errno == EINTR))
			continue;
		else
			exit(EXIT_FAILURE);
	} while (1);
}

int main(int argc, char **argv)
{

#if HAVE_DECL_PTRACE_SETOPTIONS && HAVE_DECL_PTRACE_O_TRACEVFORKDONE
	int exit_status;

	_parse_opts(argc, argv);

	if (fp == NULL) {
		fp = stderr;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, psync) == -1) {
		tst_resm(TBROK | TERRNO, "socketpair() failed");
	} else {

		child = fork();
		if (child == -1) {
			tst_resm(TBROK | TERRNO, "fork() failed");
		} else if (child == 0) {

			int rc = EXIT_FAILURE;

			tst_sig(FORK, DEF_HANDLER, child_cleanup);

			if (close(psync[1])) {
				tst_resm(TBROK, "close(psync[1]) failed)");
			} else {
				/* sleep until the parent wakes us up */
				await_mutex(psync[0]);
				rc = do_vfork(num_vforks);
			}
			_exit(rc);

		} else {

			tst_sig(FORK, kill_child, parent_cleanup);

			close(psync[0]);

			/* Set up ptrace */
			if (ptrace(PTRACE_ATTACH, child, NULL, NULL) == -1) {
				tst_brkm(TBROK | TERRNO,
					 NULL, "ptrace(ATTACH) failed");
			}
			if (waitpid(child, NULL, 0) != child) {
				tst_resm(TBROK | TERRNO, "waitpid(%d) failed",
					 child);
				kill_child();
			} else {

				if (ptrace(PTRACE_SETOPTIONS, child, NULL,
					   PTRACE_O_TRACEVFORK) == -1) {
					tst_resm(TINFO | TERRNO,
						 "ptrace(PTRACE_SETOPTIONS) "
						 "failed.");
				}
				if (ptrace(PTRACE_CONT, child, NULL, NULL) ==
				    -1) {
					tst_resm(TINFO | TERRNO,
						 "ptrace(PTRACE_CONT) failed.");
				}

				send_mutex(psync[1]);

				close(psync[1]);

				tst_resm(TINFO, "Child spawn's pid=%d", child);
				fprintf(fp, "%d\n", child);
				fflush(fp);

				exit_status = do_trace(child, ++num_vforks);

				tst_resm(exit_status == 0 ? TPASS : TFAIL,
					 "do_trace %s",
					 (exit_status ==
					  0 ? "succeeded" : "failed"));

				parent_cleanup();

			}

		}

	}

#else
	tst_resm(TCONF, "System doesn't support have required ptrace "
		 "capabilities.");
#endif
	tst_resm(TINFO, "Exiting...");
	tst_exit();

}
