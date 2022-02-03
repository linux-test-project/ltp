// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 CTERA Networks. All Rights Reserved.
 *
 * Started by Amir Goldstein <amir73il@gmail.com>
 * based on reproducer from Ivan Delalande <colona@arista.com>
 */

/*\
 * [Description]
 * Test opening files after receiving IN_DELETE.
 *
 * Kernel v5.13 has a regression allowing files to be open after IN_DELETE.
 *
 * The problem has been fixed by commit:
 *  a37d9a17f099 "fsnotify: invalidate dcache before IN_DELETE event".
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

/* Number of files to test */
#define CHURN_FILES 9999

#define EVENT_MAX 32
/* Size of the event structure, not including the name */
#define EVENT_SIZE	(sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(EVENT_MAX * (EVENT_SIZE + 16))

static pid_t pid;

static char event_buf[EVENT_BUF_LEN];

static void churn(void)
{
	char path[10];
	int i;

	for (i = 0; i <= CHURN_FILES; ++i) {
		snprintf(path, sizeof(path), "%d", i);
		SAFE_FILE_PRINTF(path, "1");
		SAFE_UNLINK(path);
	}
}

static void verify_inotify(void)
{
	int nevents = 0, opened = 0;
	struct inotify_event *event;
	int inotify_fd;

	pid = SAFE_FORK();
	if (pid == 0) {
		churn();
		return;
	}

	inotify_fd = SAFE_MYINOTIFY_INIT();
	SAFE_MYINOTIFY_ADD_WATCH(inotify_fd, ".", IN_DELETE);

	while (!opened && nevents < CHURN_FILES) {
		int i, fd, len;

		len = SAFE_READ(0, inotify_fd, event_buf, EVENT_BUF_LEN);

		for (i = 0; i < len; i += EVENT_SIZE + event->len) {
			event = (struct inotify_event *)&event_buf[i];

			if (!(event->mask & IN_DELETE))
				continue;

			nevents++;

			/* Open file after IN_DELETE should fail */
			fd = open(event->name, O_RDONLY);
			if (fd < 0)
				continue;

			tst_res(TFAIL, "File %s opened after IN_DELETE", event->name);
			SAFE_CLOSE(fd);
			opened = 1;
			break;
		}
	}

	SAFE_CLOSE(inotify_fd);

	if (!nevents)
		tst_res(TFAIL, "Didn't get any IN_DELETE events");
	else if (!opened)
		tst_res(TPASS, "Got %d IN_DELETE events", nevents);

	/* Kill the child creating / deleting files and wait for it */
	SAFE_KILL(pid, SIGKILL);
	pid = 0;
	SAFE_WAIT(NULL);
}

static void cleanup(void)
{
	if (pid) {
		SAFE_KILL(pid, SIGKILL);
		SAFE_WAIT(NULL);
	}
}

static struct tst_test test = {
	.timeout = 10,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.cleanup = cleanup,
	.test_all = verify_inotify,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a37d9a17f099"},
		{}
	}
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
