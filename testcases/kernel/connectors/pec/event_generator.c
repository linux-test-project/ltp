/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2008 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Li Zefan <lizf@cn.fujitsu.com>                                     */
/*                                                                            */
/******************************************************************************/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"

#define DEFAULT_EVENT_NUM       1

unsigned long nr_event = DEFAULT_EVENT_NUM;

uid_t ltp_uid;
gid_t ltp_gid;
const char *ltp_user = "nobody";

char **exec_argv;

void (*gen_event) (void);

/*
 * Show the usage
 *
 * @status: the exit status
 */
static void usage(int status)
{
	FILE *stream = (status ? stderr : stdout);

	fprintf(stream,
		"Usage: event_generator -e fork|exit|exec|uid|gid [-n nr_event]\n");

	exit(status);
}

/*
 * Generate exec event.
 *
 * We can't just exec nr_event times, because the current process image
 * will be replaced with the new process image, so we use enviroment
 * viriable as event counters, as it will be inherited after exec.
 */
static void gen_exec(void)
{
	char *val;
	char buf[10];
	unsigned long nr_exec;

	/* get the event counter */
	val = getenv("NR_EXEC");
	if (!val) {
		nr_exec = 0;
		setenv("NR_EXEC", "1", 1);
	} else {
		nr_exec = atoi(val);
		snprintf(buf, 10, "%lu", nr_exec + 1);
		setenv("NR_EXEC", buf, 1);
	}

	/* stop generate exec event */
	if (nr_exec >= nr_event)
		return;

	/* fflush is needed before exec */
	printf("exec pid: %d\n", getpid());
	fflush(stdout);

	execv(exec_argv[0], exec_argv);
}

/*
 * Generate fork event.
 */
static inline void gen_fork(void)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		printf("fork parent: %d, child: %d\n", getppid(), getpid());
		exit(0);
	} else if (pid < 0) {
		fprintf(stderr, "fork() failed\n");
		exit(1);
	} else {		/* Parent should wait for the child */
		wait(&status);
	}
}

/**
 * Generate exit event
 */
static inline void gen_exit(void)
{
	pid_t pid;

	pid = fork();
	if (pid == 0) {
		printf("exit pid: %d exit_code: %d\n", getpid(), 0);
		exit(0);
	} else if (pid < 0) {
		fprintf(stderr, "fork() failed\n");
		exit(1);
	}
}

/*
 * Generate uid event.
 */
static inline void gen_uid(void)
{
	setuid(ltp_uid);
	printf("uid pid: %d euid: %d\n", getpid(), ltp_uid);
}

/*
 * Generate gid event.
 */
static inline void gen_gid(void)
{
	setgid(ltp_gid);
	printf("gid pid: %d egid: %d\n", getpid(), ltp_gid);
}

/*
 * Read option from user input.
 *
 * @argc: number of arguments
 * @argv: argument list
 */
static void process_options(int argc, char **argv)
{
	int c;
	char *end;

	while ((c = getopt(argc, argv, "e:n:h")) != -1) {
		switch (c) {
			/* which event to generate */
		case 'e':
			if (!strcmp(optarg, "exec"))
				gen_event = gen_exec;
			else if (!strcmp(optarg, "fork"))
				gen_event = gen_fork;
			else if (!strcmp(optarg, "exit"))
				gen_event = gen_exit;
			else if (!strcmp(optarg, "uid"))
				gen_event = gen_uid;
			else if (!strcmp(optarg, "gid"))
				gen_event = gen_gid;
			else {
				fprintf(stderr, "wrong -e argument!");
				exit(1);
			}
			break;
			/* number of event to generate */
		case 'n':
			nr_event = strtoul(optarg, &end, 10);
			if (*end != '\0' || nr_event == 0) {
				fprintf(stderr, "wrong -n argument!");
				exit(1);
			}
			break;
			/* help */
		case 'h':
			usage(0);
		default:
			fprintf(stderr, "unknown option!\n");
			usage(1);
		}
	}

	if (!gen_event) {
		fprintf(stderr, "no event type specified!\n");
		usage(1);
	}
}

int main(int argc, char **argv)
{
	unsigned long i;
	struct passwd *ent;

	process_options(argc, argv);

	ent = getpwnam(ltp_user);
	if (ent == NULL) {
		fprintf(stderr, "can't get password entry for %s", ltp_user);
		exit(1);
	}
	ltp_uid = ent->pw_uid;
	ltp_gid = ent->pw_gid;

	signal(SIGCHLD, SIG_IGN);

	/* special processing for gen_exec, see comments above gen_exec() */
	if (gen_event == gen_exec) {
		exec_argv = argv;

		gen_exec();

		/* won't reach here */
		return 0;
	}

	/* other events */
	for (i = 0; i < nr_event; i++)
		gen_event();

	return 0;
}
