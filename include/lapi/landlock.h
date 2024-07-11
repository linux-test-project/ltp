// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LAPI_LANDLOCK_H__
#define LAPI_LANDLOCK_H__

#include "config.h"

#ifdef HAVE_LINUX_LANDLOCK_H
# include <linux/landlock.h>
#endif

#ifndef HAVE_STRUCT_LANDLOCK_RULESET_ATTR
struct landlock_ruleset_attr
{
	uint64_t handled_access_fs;
	uint64_t handled_access_net;
};
#endif

#ifndef HAVE_STRUCT_LANDLOCK_PATH_BENEATH_ATTR
struct landlock_path_beneath_attr
{
	uint64_t allowed_access;
	int32_t parent_fd;
} __attribute__((packed));
#endif

#ifndef HAVE_ENUM_LANDLOCK_RULE_TYPE
enum landlock_rule_type
{
	LANDLOCK_RULE_PATH_BENEATH = 1,
	LANDLOCK_RULE_NET_PORT,
};
#endif

#ifndef HAVE_STRUCT_LANDLOCK_NET_PORT_ATTR
struct landlock_net_port_attr
{
	uint64_t allowed_access;
	uint64_t port;
};
#endif

#ifndef LANDLOCK_CREATE_RULESET_VERSION
# define LANDLOCK_CREATE_RULESET_VERSION	(1U << 0)
#endif

#ifndef LANDLOCK_ACCESS_FS_EXECUTE
# define LANDLOCK_ACCESS_FS_EXECUTE			(1ULL << 0)
#endif

#ifndef LANDLOCK_ACCESS_FS_WRITE_FILE
# define LANDLOCK_ACCESS_FS_WRITE_FILE		(1ULL << 1)
#endif

#ifndef LANDLOCK_ACCESS_FS_READ_FILE
# define LANDLOCK_ACCESS_FS_READ_FILE		(1ULL << 2)
#endif

#ifndef LANDLOCK_ACCESS_FS_READ_DIR
# define LANDLOCK_ACCESS_FS_READ_DIR		(1ULL << 3)
#endif

#ifndef LANDLOCK_ACCESS_FS_REMOVE_DIR
# define LANDLOCK_ACCESS_FS_REMOVE_DIR		(1ULL << 4)
#endif

#ifndef LANDLOCK_ACCESS_FS_REMOVE_FILE
# define LANDLOCK_ACCESS_FS_REMOVE_FILE		(1ULL << 5)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_CHAR
# define LANDLOCK_ACCESS_FS_MAKE_CHAR		(1ULL << 6)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_DIR
# define LANDLOCK_ACCESS_FS_MAKE_DIR		(1ULL << 7)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_REG
# define LANDLOCK_ACCESS_FS_MAKE_REG		(1ULL << 8)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_SOCK
# define LANDLOCK_ACCESS_FS_MAKE_SOCK		(1ULL << 9)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_FIFO
# define LANDLOCK_ACCESS_FS_MAKE_FIFO		(1ULL << 10)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_BLOCK
# define LANDLOCK_ACCESS_FS_MAKE_BLOCK		(1ULL << 11)
#endif

#ifndef LANDLOCK_ACCESS_FS_MAKE_SYM
# define LANDLOCK_ACCESS_FS_MAKE_SYM		(1ULL << 12)
#endif

#ifndef LANDLOCK_ACCESS_FS_REFER
# define LANDLOCK_ACCESS_FS_REFER			(1ULL << 13)
#endif

#ifndef LANDLOCK_ACCESS_FS_TRUNCATE
# define LANDLOCK_ACCESS_FS_TRUNCATE		(1ULL << 14)
#endif

#ifndef LANDLOCK_ACCESS_FS_IOCTL_DEV
# define LANDLOCK_ACCESS_FS_IOCTL_DEV		(1ULL << 15)
#endif

#ifndef LANDLOCK_ACCESS_NET_BIND_TCP
# define LANDLOCK_ACCESS_NET_BIND_TCP		(1ULL << 0)
#endif

#ifndef LANDLOCK_ACCESS_NET_CONNECT_TCP
# define LANDLOCK_ACCESS_NET_CONNECT_TCP	(1ULL << 1)
#endif

#endif
