// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Fill the watch queue and wait for a notification loss.
 */

#define _GNU_SOURCE

#include <unistd.h>
#include "tst_test.h"
#include "lapi/keyctl.h"
#include "common.h"

#define WATCH_QUEUE_NOTE_SIZE 128

static int data_lost;
static key_serial_t key;
static int fd;

static void saw_data_loss(struct watch_notification *n,
			  LTP_ATTRIBUTE_UNUSED size_t len, unsigned int wtype)
{
	if (wtype != WATCH_TYPE_META)
		return;

	if (n->subtype == WATCH_META_LOSS_NOTIFICATION)
		data_lost = 1;
}

static void setup(void)
{
	fd = wqueue_watch(1, &wqueue_filter);
	key = wqueue_add_key(fd);
}

static void run(void)
{
	int i, iterations;

	iterations = (getpagesize() / WATCH_QUEUE_NOTE_SIZE) * 2;
	for (i = 0; i < iterations; i++)
		keyctl(KEYCTL_UPDATE, key, "b", 1);

	data_lost = 0;
	while (!data_lost)
		wqueue_consumer(fd, saw_data_loss);

	if (data_lost)
		tst_res(TPASS, "Meta loss notification received");
	else
		tst_res(TFAIL, "Event not recognized");
}

static void cleanup(void)
{
	keyctl(KEYCTL_REVOKE, key);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
