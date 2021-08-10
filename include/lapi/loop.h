// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef LAPI_LOOP_H__
#define LAPI_LOOP_H__

#include "config.h"
#include <linux/types.h>
#include <linux/loop.h>

#ifndef LO_FLAGS_PARTSCAN
# define LO_FLAGS_PARTSCAN 8
#endif

#ifndef LO_FLAGS_DIRECT_IO
# define LO_FLAGS_DIRECT_IO 16
#endif

#ifndef LOOP_SET_CAPACITY
# define LOOP_SET_CAPACITY 0x4C07
#endif

#ifndef LOOP_SET_DIRECT_IO
# define LOOP_SET_DIRECT_IO 0x4C08
#endif

#ifndef LOOP_SET_BLOCK_SIZE
# define LOOP_SET_BLOCK_SIZE 0x4C09
#endif

#ifndef LOOP_CONFIGURE
# define LOOP_CONFIGURE 0x4C0A
#endif

#ifndef HAVE_STRUCT_LOOP_CONFIG
/*
 * struct loop_config - Complete configuration for a loop device.
 * @fd: fd of the file to be used as a backing file for the loop device.
 * @block_size: block size to use; ignored if 0.
 * @info: struct loop_info64 to configure the loop device with.
 *
 * This structure is used with the LOOP_CONFIGURE ioctl, and can be used to
 * atomically setup and configure all loop device parameters at once.
 */
struct loop_config {
	__u32			fd;
	__u32                   block_size;
	struct loop_info64	info;
	__u64			__reserved[8];
};
#endif

#endif /* LAPI_LOOP_H__ */
