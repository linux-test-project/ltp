// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test if key watch removal is correctly recognized by watch queue.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/keyctl.h"
#include "common.h"

static void saw_watch_removal(struct watch_notification *n,
			      LTP_ATTRIBUTE_UNUSED size_t len,
			      unsigned int wtype)
{
	if (wtype != WATCH_TYPE_META)
		return;

	if (n->subtype == WATCH_META_REMOVAL_NOTIFICATION)
		tst_res(TPASS, "Meta removal notification received");
	else
		tst_res(TFAIL, "Event not recognized");
}

static void run(void)
{
	int fd;
	key_serial_t key;

	fd = wqueue_watch(256, &wqueue_filter);
	key = wqueue_add_key(fd);

	/* if watch_id = -1 key is removed from the watch queue */
	keyctl(KEYCTL_WATCH_KEY, key, fd, -1);
	wqueue_consumer(fd, saw_watch_removal);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
};
