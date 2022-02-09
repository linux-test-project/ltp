// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Huawei.  All Rights Reserved.
 *
 * Started by nixiaoming <nixiaoming@huawei.com>
 */

/*\
 * [Description]
 * After fanotify_init adds flags FAN_REPORT_TID,
 * check whether the program can accurately identify which thread id
 * in the multithreaded program triggered the event.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "tst_test.h"
#include "tst_safe_pthread.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define gettid() syscall(SYS_gettid)
static int tid;

static int fan_report_tid_unsupported;

static void *thread_create_file(void *arg LTP_ATTRIBUTE_UNUSED)
{
	char tid_file[64] = {0};

	tid = gettid();
	snprintf(tid_file, sizeof(tid_file), "test_tid_%d",  tid);
	SAFE_FILE_PRINTF(tid_file, "1");

	pthread_exit(0);
}

static unsigned int tcases[] = {
	FAN_CLASS_NOTIF,
	FAN_CLASS_NOTIF | FAN_REPORT_TID
};

static void test01(unsigned int i)
{
	pthread_t p_id;
	struct fanotify_event_metadata event;
	int fd_notify;
	int tgid = getpid();

	tst_res(TINFO, "Test #%u: %s FAN_REPORT_TID: tgid=%d, tid=%d, event.pid=%d",
			i, (tcases[i] & FAN_REPORT_TID) ? "with" : "without",
			tgid, tid, event.pid);

	if (fan_report_tid_unsupported && (tcases[i] & FAN_REPORT_TID)) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_TID, fan_report_tid_unsupported);
		return;
	}

	fd_notify = SAFE_FANOTIFY_INIT(tcases[i], 0);

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD,
			FAN_ALL_EVENTS | FAN_EVENT_ON_CHILD, AT_FDCWD, ".");

	SAFE_PTHREAD_CREATE(&p_id, NULL, thread_create_file, NULL);

	SAFE_READ(0, fd_notify, &event, sizeof(struct fanotify_event_metadata));

	if ((tcases[i] & FAN_REPORT_TID) && event.pid == tid)
		tst_res(TPASS, "event.pid == tid");
	else if (!(tcases[i] & FAN_REPORT_TID) && event.pid == tgid)
		tst_res(TPASS, "event.pid == tgid");
	else
		tst_res(TFAIL, "unexpected event.pid value");

	if (event.fd != FAN_NOFD)
		SAFE_CLOSE(event.fd);
	SAFE_CLOSE(fd_notify);
	SAFE_PTHREAD_JOIN(p_id, NULL);
}

static void setup(void)
{
	fan_report_tid_unsupported = fanotify_init_flags_supported_by_kernel(FAN_REPORT_TID);
}

static struct tst_test test = {
	.setup = setup,
	.test = test01,
	.tcnt =  ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.needs_root = 1,
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
