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
 * received before exitting.
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
#include "test.h"
#include "config.h"
#include "../../syscalls/ptrace/ptrace.h"

#define str_expand(s) str(s)
#define str(s) #s

#define debug(s) \
perror("ERROR at " __FILE__ ":" str_expand(__LINE__) ": " s )

int   TST_TOTAL = 1;
char* TCID = "vfork";

/* for signal handlers */
void do_nothing(int sig)
{
	return;
}

int do_vfork(int count)
{
	pid_t child;

	signal(SIGCHLD, SIG_IGN); /* Avoid waiting for vfork'd processes */
	while (count) {
		child = vfork();
		if (child == 0)
			_exit(0);
		else if (child > 0)
			count--;
		else {
			debug("vfork(): ");
			exit(EXIT_FAILURE);
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
	} while(1);
}

void usage(const char *prog)
{
	fprintf(stderr, "%s [-s [NUM]] [-p] [NUM]\n\n"
		"\t-s NUM\t\tSleep for NUM seconds. [Default: 1 second]\n"
		"\t\t\t\tSuffixes ms, us, s, m, and h correspond to\n"
		"\t\t\t\tmilliseconds, microseconds, seconds [Default],\n"
		"\t\t\t\tminutes, and hours respectively.\n\n"
		"\t-p\t\tPause.\n\n"
		"\tNUM\t\tExecute vfork NUM times.\n", prog);
}

void parse_opts(int argc, char **argv)
{
	int opt;
	char *units;
	unsigned long long duration = 1U;

	sleep_duration.tv_sec = 0U;
	sleep_duration.tv_nsec = 0U;

	while((opt = getopt(argc, argv, "ps::")) != -1) {
		switch(opt) {
		case 'p':
			do_pause = 1;
			break;
		case 's':
			if (optarg == NULL) {
				sleep_duration.tv_sec = 1;
				do_sleep = 1;
				break;
			}
			opt = sscanf(optarg, "%Ld%as",
				     &duration, &units);
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
				fprintf(stderr, "%s: Unrecognized time units: %s\n", argv[0], units);
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			do_sleep = 1;
			break;
		default:
			usage(argv[0]);
			exit(EXIT_FAILURE);
			break;
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
	/*dump_siginfo(gchild, &info);*/
	if ((info.si_code != 0) || (info.si_signo != SIGSTOP))
		return 0;

	fprintf(stderr, "Grandchild spawn's pid=%d", gchild);
	printf("\t%d\n", gchild);
	if (do_pause)
		pause();
	if (do_sleep)
		sleepy_time();
	if (ptrace(PTRACE_DETACH, gchild, NULL, NULL) == -1)
		debug("ptrace(): ");
	return -1; /* don't wait for gchild */
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

		/*printf("waiting for %d processes to exit\n", num_children);*/
		process = waitpid(-1, &status, WUNTRACED);
		if (process < 1)
			continue;
		/*dump_status(process, status);*/
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
	} while(1);
}

int
main(int argc, char** argv)
{

#if defined(HAVE_DECL_PTRACE_SETOPTIONS) && defined(HAVE_DECL_PTRACE_O_TRACEVFORK)
	pid_t child;
	int exit_status, psync[2];

	parse_opts(argc, argv);

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, psync) == -1) {
		debug("socketpair(): ");
		exit(EXIT_FAILURE);
	}

	child = fork();
	if (child == -1) {
		tst_resm(TBROK | TERRNO, "fork() failed");
	} else if (child == 0) {
		close(psync[1]);

		await_mutex(psync[0]); /* sleep until parent wakes us up */

		close(psync[0]);
		_exit(do_vfork(num_vforks));
	} else {
		close(psync[0]);

		/* Set up ptrace */
		if (ptrace(PTRACE_ATTACH, child, NULL, NULL) == -1) {
			debug("ptrace(ATTACH): ");
			kill(child, SIGKILL);
			exit(EXIT_FAILURE);
		}
		if (waitpid(child, NULL, 0) != child) {
			debug("waitpid(): ");
			exit(EXIT_FAILURE);
		}
		if (ptrace(PTRACE_SETOPTIONS, child, NULL,
			   PTRACE_O_TRACEVFORK) == -1)
			debug("ptrace(SETOPTIONS): ");
		if (ptrace(PTRACE_CONT, child, NULL, NULL) == -1)
			debug("ptrace(CONT): ");

		send_mutex(psync[1]);

		close(psync[1]);
		fprintf(stderr, "Child spawn's pid=%d", child);
		printf("%d\n", child);

		exit_status = do_trace(child, ++num_vforks);

		tst_resm(exit_status == 0 ? TPASS : TFAIL, "do_trace exit status %s",
			 (exit_status == 0 ? "succeeded" : "failed"));

	}
#else
	tst_resm(TCONF, "System doesn't support have required ptrace "
			"capabilities.\n");
#endif
	tst_exit();

}
