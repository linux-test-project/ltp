// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 *
 * This file is created to resolve conflicts between user space and kernel
 * space fctnl.h declaration. linux/watch_queue.h is not handled, since
 * user space fcntl.h redefines kernel space structures.
 */

#ifndef LAPI_WATCH_QUEUE_H__
#define LAPI_WATCH_QUEUE_H__

#include <stdint.h>
#include "lapi/ioctl.h"
#include "lapi/fcntl.h"

#define O_NOTIFICATION_PIPE	O_EXCL	/* Parameter to pipe2() selecting notification pipe */

#define IOC_WATCH_QUEUE_SET_SIZE	_IO('W', 0x60)	/* Set the size in pages */
#define IOC_WATCH_QUEUE_SET_FILTER	_IO('W', 0x61)	/* Set the filter */

enum watch_notification_type {
	WATCH_TYPE_META		= 0,	/* Special record */
	WATCH_TYPE_KEY_NOTIFY	= 1,	/* Key change event notification */
	WATCH_TYPE__NR		= 2
};

enum watch_meta_notification_subtype {
	WATCH_META_REMOVAL_NOTIFICATION	= 0,	/* Watched object was removed */
	WATCH_META_LOSS_NOTIFICATION	= 1,	/* Data loss occurred */
};

/*
 * Notification record header.  This is aligned to 64-bits so that subclasses
 * can contain __u64 fields.
 */
struct watch_notification {
	uint32_t			type:24;	/* enum watch_notification_type */
	uint32_t			subtype:8;	/* Type-specific subtype (filterable) */
	uint32_t			info;
#define WATCH_INFO_LENGTH	0x0000007f	/* Length of record */
#define WATCH_INFO_LENGTH__SHIFT 0
#define WATCH_INFO_ID		0x0000ff00	/* ID of watchpoint */
#define WATCH_INFO_ID__SHIFT	8
#define WATCH_INFO_TYPE_INFO	0xffff0000	/* Type-specific info */
#define WATCH_INFO_TYPE_INFO__SHIFT 16
#define WATCH_INFO_FLAG_0	0x00010000	/* Type-specific info, flag bit 0 */
#define WATCH_INFO_FLAG_1	0x00020000	/* ... */
#define WATCH_INFO_FLAG_2	0x00040000
#define WATCH_INFO_FLAG_3	0x00080000
#define WATCH_INFO_FLAG_4	0x00100000
#define WATCH_INFO_FLAG_5	0x00200000
#define WATCH_INFO_FLAG_6	0x00400000
#define WATCH_INFO_FLAG_7	0x00800000
};

/*
 * Notification filtering rules (IOC_WATCH_QUEUE_SET_FILTER).
 */
struct watch_notification_type_filter {
	uint32_t	type;			/* Type to apply filter to */
	uint32_t	info_filter;		/* Filter on watch_notification::info */
	uint32_t	info_mask;		/* Mask of relevant bits in info_filter */
	uint32_t	subtype_filter[8];	/* Bitmask of subtypes to filter on */
};

struct watch_notification_filter {
	uint32_t	nr_filters;		/* Number of filters */
	uint32_t	__reserved;		/* Must be 0 */
	struct watch_notification_type_filter filters[];
};


/*
 * Extended watch removal notification.  This is used optionally if the type
 * wants to indicate an identifier for the object being watched, if there is
 * such.  This can be distinguished by the length.
 *
 * type -> WATCH_TYPE_META
 * subtype -> WATCH_META_REMOVAL_NOTIFICATION
 */
struct watch_notification_removal {
	struct watch_notification watch;
	uint64_t	id;		/* Type-dependent identifier */
};

/*
 * Type of key/keyring change notification.
 */
enum key_notification_subtype {
	NOTIFY_KEY_INSTANTIATED	= 0, /* Key was instantiated (aux is error code) */
	NOTIFY_KEY_UPDATED	= 1, /* Key was updated */
	NOTIFY_KEY_LINKED	= 2, /* Key (aux) was added to watched keyring */
	NOTIFY_KEY_UNLINKED	= 3, /* Key (aux) was removed from watched keyring */
	NOTIFY_KEY_CLEARED	= 4, /* Keyring was cleared */
	NOTIFY_KEY_REVOKED	= 5, /* Key was revoked */
	NOTIFY_KEY_INVALIDATED	= 6, /* Key was invalidated */
	NOTIFY_KEY_SETATTR	= 7, /* Key's attributes got changed */
};

/*
 * Key/keyring notification record.
 * - watch.type = WATCH_TYPE_KEY_NOTIFY
 * - watch.subtype = enum key_notification_type
 */
struct key_notification {
	struct watch_notification watch;
	uint32_t	key_id;		/* The key/keyring affected */
	uint32_t	aux;		/* Per-type auxiliary data */
};

#endif /* LAPI_WATCH_QUEUE_H__ */
