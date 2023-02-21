// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) The GNU Toolchain Authors.
 * Copyright (c) 2023 Wei Gao <wegao@suse.com>
 *
 */

/*\
 * [Description]
 *
 * Test for NULL pointer dereference in mq_notify(CVE-2021-38604)
 *
 * References links:
 * - https://sourceware.org/bugzilla/show_bug.cgi?id=28213
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "tst_test.h"
#include "tst_safe_posix_ipc.h"

static mqd_t m = -1;
static const char msg[] = "hello";

static void try_null_dereference_cb(union sigval sv)
{
	char buf[sizeof(msg)];

	(void)sv;

	TST_EXP_VAL((size_t) mq_receive(m, buf, sizeof(buf), NULL),
	            sizeof(buf));
	TST_EXP_PASS(memcmp(buf, msg, sizeof(buf)));

	exit(0);
}

static void try_null_dereference(void)
{
	struct sigevent sev;

	memset(&sev, '\0', sizeof(sev));
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = try_null_dereference_cb;

	/* Step 1: Register & unregister notifier.
	 * Helper thread should receive NOTIFY_REMOVED notification.
	 * In a vulnerable version of glibc, NULL pointer dereference follows.
	 */
	TST_EXP_PASS(mq_notify(m, &sev));
	TST_EXP_PASS(mq_notify(m, NULL));

	/* Step 2: Once again, register notification.
	 * Try to send one message.
	 * Test is considered successful, if the callback does exit (0).
	 */
	TST_EXP_PASS(mq_notify(m, &sev));
	TST_EXP_PASS(mq_send(m, msg, sizeof(msg), 1));

	/* Wait... */
	pause();
}

static void do_test(void)
{
	static const char m_name[] = "/ltp_mq_notify03";
	struct mq_attr m_attr;

	memset(&m_attr, '\0', sizeof(m_attr));
	m_attr.mq_maxmsg = 1;
	m_attr.mq_msgsize = sizeof(msg);

	m = SAFE_MQ_OPEN(m_name,
			O_RDWR | O_CREAT | O_EXCL,
			0600,
			&m_attr);

	TST_EXP_PASS(mq_unlink(m_name));

	try_null_dereference();
}


static struct tst_test test = {
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"glibc-git", "b805aebd42"},
		{"CVE", "2021-38604"},
		{}
	},
	.needs_root = 1,
};
