// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test if keyctl revoke is correctly recognized by watch queue.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/keyctl.h"
#include "common.h"

static void saw_key_revoked(struct watch_notification *n, size_t len,
			    unsigned int wtype)
{
	if (wqueue_key_event(n, len, wtype, NOTIFY_KEY_REVOKED))
		tst_res(TPASS, "keyctl revoke has been recognized");
	else
		tst_res(TFAIL, "keyctl revoke has not been recognized");
}

static void run(void)
{
	int fd;
	key_serial_t key;

	fd = wqueue_watch(256, &wqueue_filter);
	key = wqueue_add_key(fd);

	keyctl(KEYCTL_REVOKE, key);
	wqueue_consumer(fd, saw_key_revoked);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
};
