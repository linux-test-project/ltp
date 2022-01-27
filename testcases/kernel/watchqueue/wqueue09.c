// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Fill the watch queue and wait for a notification loss.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/keyctl.h"
#include "common.h"

static int data_lost;

static void saw_data_loss(struct watch_notification *n,
			  LTP_ATTRIBUTE_UNUSED size_t len, unsigned int wtype)
{
	if (wtype != WATCH_TYPE_META)
		return;

	if (n->subtype == WATCH_META_LOSS_NOTIFICATION)
		data_lost = 1;
}

static void run(void)
{
	int fd;
	key_serial_t key;

	fd = wqueue_watch(1, &wqueue_filter);

	key = wqueue_add_key(fd);
	keyctl(KEYCTL_UPDATE, key, "b", 1);
	keyctl(KEYCTL_REVOKE, key);

	data_lost = 0;
	while (!data_lost)
		wqueue_consumer(fd, saw_data_loss);

	SAFE_CLOSE(fd);

	if (data_lost)
		tst_res(TPASS, "Meta loss notification received");
	else
		tst_res(TFAIL, "Event not recognized");
}

static struct tst_test test = {
	.test_all = run,
};
