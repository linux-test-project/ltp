// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2008 FUJITSU LIMITED
 * Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
 *
 * Author: Li Zefan <lizf@cn.fujitsu.com>
 *
 * Generate a specified process event (fork, exec, uid, gid or exit).
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

extern struct tst_test *tst_test;
static struct tst_test test = {
	.forks_child = 1
};

#define DEFAULT_EVENT_NUM       1

unsigned long nr_event = DEFAULT_EVENT_NUM;

uid_t ltp_uid;
gid_t ltp_gid;
const char *ltp_user = "nobody";

char **exec_argv;

void (*gen_event) (void);
static void usage(int status) LTP_ATTRIBUTE_NORETURN;

/*
 * Show the usage
 *
 * @param status the exit status
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
 * will be replaced with the new process image, so we use environment
 * variable as event counters, as it will be inherited after exec.
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

	/* Note: This expects the full path to self in exec_argv[0]! */
	SAFE_EXECVP(exec_argv[0], exec_argv);
}

/*
 * Generate fork event.
 */
static inline void gen_fork(void)
{
	/* The actual fork is already done in main */
	printf("fork parent: %d, child: %d\n", getppid(), getpid());
}

/**
 * Generate exit event
 */
static inline void gen_exit(void)
{
	/* exit_signal will always be SIGCHLD, if the process terminates cleanly */
	printf("exit pid: %d exit_code: %d exit_signal: %d\n",
	       getpid(), 0, SIGCHLD);
	/* exit is called by main already */
}

/*
 * Generate uid event.
 */
static inline void gen_uid(void)
{
	SAFE_SETUID(ltp_uid);
	printf("uid pid: %d euid: %d ruid: %d\n", getpid(), ltp_uid, ltp_uid);
}

/*
 * Generate gid event.
 */
static inline void gen_gid(void)
{
	SAFE_SETGID(ltp_gid);
	printf("gid pid: %d egid: %d rgid: %u\n", getpid(), ltp_gid, ltp_gid);
}

/*
 * Read option from user input.
 *
 * @param argc number of arguments
 * @param argv argument list
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

	tst_test = &test;

	process_options(argc, argv);

	ent = getpwnam(ltp_user);
	if (ent == NULL) {
		fprintf(stderr, "can't get password entry for %s", ltp_user);
		exit(1);
	}
	ltp_uid = ent->pw_uid;
	ltp_gid = ent->pw_gid;

	/* special processing for gen_exec, see comments above gen_exec() */
	if (gen_event == gen_exec) {
		exec_argv = argv;

		gen_exec();

		/* won't reach here */
		return 0;
	}

	/* other events */
	for (i = 0; i < nr_event; i++) {
		pid_t pid;
		int status;

		pid = SAFE_FORK();
		if (pid == 0) {
			gen_event();
			exit(0);
		} else {
			if (pid != SAFE_WAITPID(pid, &status, 0)) {
				fprintf(stderr,
				        "Child process did not terminate as expected\n");
				return 1;
			}
			if (WEXITSTATUS(status) != 0) {
				fprintf(stderr, "Child process did not terminate with 0\n");
				return 1;
			}
		}
	}

	return 0;
}
