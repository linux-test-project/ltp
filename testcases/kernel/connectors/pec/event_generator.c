// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2008 FUJITSU LIMITED
 * Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
 * Author: Li Zefan <lizf@cn.fujitsu.com>
 */

/*\
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

static uid_t ltp_uid;
static gid_t ltp_gid;
static const char *ltp_user = "nobody";
static char *prog_name;

static int checkpoint_id = -1;
static int nr_event = 1;

static void (*gen_event)(void);

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
		"Usage: event_generator -e fork|exit|exec|uid|gid [-n nr_event] [-c checkpoint_id]\n");

	exit(status);
}

/*
 * Generate exec event.
 */
static void gen_exec(void)
{
	char buf[10];

	/* fflush is needed before exec */
	printf("exec pid: %d\n", getpid());
	fflush(stdout);

	/*
	 * Decrease number of events to generate.
	 * Don't pass checkpoint_id here. It is only used for synchronizing with
	 * the shell script, before the first exec.
	 */
	sprintf(buf, "%u", nr_event - 1);
	SAFE_EXECLP(prog_name, prog_name, "-e", "exec", "-n", buf, NULL);
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

	while ((c = getopt(argc, argv, "e:n:c:h")) != -1) {
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
			if (tst_parse_int(optarg, &nr_event, 0, INT_MAX)) {
				fprintf(stderr, "invalid value for nr_event");
				usage(1);
			}
			break;
		case 'c':
			if (tst_parse_int(optarg, &checkpoint_id, 0, INT_MAX)) {
				fprintf(stderr, "invalid value for checkpoint_id");
				usage(1);
			}
			break;
			/* help */
		case 'h':
			usage(0);
		default:
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
	int i;
	struct passwd *ent;

	prog_name = argv[0];

	tst_test = &test;

	process_options(argc, argv);

	ent = getpwnam(ltp_user);
	if (ent == NULL) {
		fprintf(stderr, "can't get password entry for %s", ltp_user);
		exit(1);
	}
	ltp_uid = ent->pw_uid;
	ltp_gid = ent->pw_gid;

	/* ready to generate events */
	if (checkpoint_id != -1) {
		tst_reinit();
		TST_CHECKPOINT_WAIT(checkpoint_id);
	}

	if (gen_event == gen_exec) {
		/*
		 * The nr_event events are generated,
		 * by recursively replacing ourself with
		 * a fresh copy, decrementing the number of events
		 * for each execution
		 */
		if (nr_event != 0)
			gen_exec();
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
			/*
			 * We need a tiny sleep here, so the kernel can generate
			 * exit events in the correct order.
			 * Otherwise it can happen, that exit events are generated
			 * out-of-order.
			 */
			if (gen_event == gen_exit)
				usleep(100);
		}
	}

	return 0;
}
