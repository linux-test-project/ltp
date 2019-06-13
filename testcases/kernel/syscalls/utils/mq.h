// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef MQ_H
#define MQ_H

#include "tst_test.h"
#include "tst_sig_proc.h"
#include "tst_safe_posix_ipc.h"

#define MAX_MSGSIZE	8192
#define MSG_LENGTH	10
#define QUEUE_NAME	"/test_mqueue"
#define QUEUE_NAME_NONBLOCK	"/test_mqueue_nonblock"

static char smsg[MAX_MSGSIZE];
static struct sigaction act;

static void cleanup_common(void)
{
	if (fd_root > 0)
		SAFE_CLOSE(fd_root);

	if (fd > 0)
		SAFE_CLOSE(fd);

	if (fd_nonblock > 0)
		SAFE_CLOSE(fd_nonblock);

	mq_unlink(QUEUE_NAME);
	mq_unlink(QUEUE_NAME_NONBLOCK);
}

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED) { }

static void setup_common(void)
{
	int i;

	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);

	cleanup_common();

	fd_root = SAFE_OPEN("/", O_RDONLY);
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0700, NULL);
	fd_nonblock = SAFE_MQ_OPEN(QUEUE_NAME_NONBLOCK, O_CREAT | O_EXCL | O_RDWR |
		O_NONBLOCK, 0700, NULL);

	for (i = 0; i < MAX_MSGSIZE; i++)
		smsg[i] = i;
}

static void cleanup_queue(mqd_t fd)
{
	int i;
	struct mq_attr mqstat;
	unsigned int prio;
	char rmsg[MAX_MSGSIZE];

	memset(&mqstat, 0, sizeof(mqstat));
	if (mq_getattr(fd, &mqstat) == -1) {
		tst_brk(TBROK|TERRNO, "mq_getattr() failed");
		return;
	}

	for (i = 0; i < mqstat.mq_curmsgs; i++) {
		tst_res(TINFO, "receive %d/%ld message", i + 1, mqstat.mq_curmsgs);
		mq_receive(fd, rmsg, MAX_MSGSIZE, &prio);
	}
}

#endif /* MQ_H */
