// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef WQUEUE_COMMON_H__
#define WQUEUE_COMMON_H__

#include <unistd.h>
#include "tst_test.h"
#include "lapi/watch_queue.h"
#include "lapi/keyctl.h"

static struct watch_notification_filter wqueue_filter = {
	.nr_filters	= 2,
	.filters = {
		[0]	= {
			.type			= WATCH_TYPE_META,
			.subtype_filter[0]	= UINT_MAX,
		},
		[1]	= {
			.type			= WATCH_TYPE_KEY_NOTIFY,
			.subtype_filter[0]	= UINT_MAX,
		},
	},
};

static inline int wqueue_key_event(struct watch_notification *n, size_t len,
				   unsigned int wtype, int type)
{
	struct key_notification *k;
	const char *msg;

	if (wtype != WATCH_TYPE_KEY_NOTIFY)
		return 0;

	if (len != sizeof(struct key_notification))
		tst_brk(TBROK, "Incorrect key message length");

	switch (n->subtype) {
	case NOTIFY_KEY_INSTANTIATED:
		msg = "instantiated";
		break;
	case NOTIFY_KEY_UPDATED:
		msg = "updated";
		break;
	case NOTIFY_KEY_LINKED:
		msg = "linked";
		break;
	case NOTIFY_KEY_UNLINKED:
		msg = "unlinked";
		break;
	case NOTIFY_KEY_CLEARED:
		msg = "cleared";
		break;
	case NOTIFY_KEY_REVOKED:
		msg = "revoked";
		break;
	case NOTIFY_KEY_INVALIDATED:
		msg = "invalidated";
		break;
	case NOTIFY_KEY_SETATTR:
		msg = "setattr";
		break;
	default:
		msg = "Invalid notification";
		break;
	};

	k = (struct key_notification *)n;
	tst_res(TINFO, "KEY %08x change=%u[%s] aux=%u", k->key_id, n->subtype, msg,
			k->aux);

	if (n->subtype == type)
		return 1;

	return 0;
}

static inline key_serial_t wqueue_add_key(int fd)
{
	key_serial_t key;

	key = add_key("user", "ltptestkey", "a", 1, KEY_SPEC_SESSION_KEYRING);
	if (key == -1)
		tst_brk(TBROK, "add_key error: %s", tst_strerrno(errno));

	SAFE_KEYCTL(KEYCTL_WATCH_KEY, key, fd, 0x01, 0);
	SAFE_KEYCTL(KEYCTL_WATCH_KEY, KEY_SPEC_SESSION_KEYRING, fd, 0x02, 0);

	return key;
}

static inline int wqueue_watch(int buf_size,
			       struct watch_notification_filter *filter)
{
	int pipefd[2];
	int fd;

	TEST(pipe2(pipefd, O_NOTIFICATION_PIPE));
	if (TST_RET) {
		switch (TST_ERR) {
		case ENOPKG:
			tst_brk(TCONF | TTERRNO, "CONFIG_WATCH_QUEUE is not set");
			break;
		case EINVAL:
			tst_brk(TCONF | TTERRNO, "O_NOTIFICATION_PIPE is not supported");
			break;
		default:
			tst_brk(TBROK | TTERRNO, "pipe2() returned %ld", TST_RET);
		}
	}

	fd = pipefd[0];

	SAFE_IOCTL(fd, IOC_WATCH_QUEUE_SET_SIZE, buf_size);
	SAFE_IOCTL(fd, IOC_WATCH_QUEUE_SET_FILTER, filter);

	return fd;
}

typedef void (*wqueue_callback)(struct watch_notification *n, size_t len,
				unsigned int wtype);

static void wqueue_consumer(int fd, wqueue_callback cb)
{
	unsigned char buffer[433], *p, *end;
	union {
		struct watch_notification n;
		unsigned char buf1[128];
	} n;
	ssize_t buf_len;

	tst_res(TINFO, "Reading watch queue events");

	buf_len = SAFE_READ(0, fd, buffer, sizeof(buffer));

	p = buffer;
	end = buffer + buf_len;
	while (p < end) {
		size_t largest, len;

		largest = end - p;
		if (largest > 128)
			largest = 128;

		if (largest < sizeof(struct watch_notification))
			tst_brk(TBROK, "Short message header: %zu", largest);

		memcpy(&n, p, largest);

		tst_res(TINFO, "NOTIFY[%03zx]: ty=%06x sy=%02x i=%08x", p - buffer,
				n.n.type, n.n.subtype, n.n.info);

		len = n.n.info & WATCH_INFO_LENGTH;
		if (len < sizeof(n.n) || len > largest)
			tst_brk(TBROK, "Bad message length: %zu/%zu", len, largest);

		cb(&n.n, len, n.n.type);

		p += len;
	}
}

#endif
