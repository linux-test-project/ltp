// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Dai Shili <daisl.fnst@cn.fujitsu.com>
 */
#ifndef LAPI_FSVERITY_H__
#define LAPI_FSVERITY_H__

#include "config.h"
#include <stdint.h>
#include <sys/ioctl.h>

#ifdef HAVE_LINUX_FSVERITY_H
#include <linux/fsverity.h>
#endif

#ifndef FS_VERITY_HASH_ALG_SHA256
# define FS_VERITY_HASH_ALG_SHA256       1
#endif

#ifndef HAVE_STRUCT_FSVERITY_ENABLE_ARG
struct fsverity_enable_arg {
	uint32_t version;
	uint32_t hash_algorithm;
	uint32_t block_size;
	uint32_t salt_size;
	uint64_t salt_ptr;
	uint32_t sig_size;
	uint32_t __reserved1;
	uint64_t sig_ptr;
	uint64_t __reserved2[11];
};
#endif

#ifndef FS_IOC_ENABLE_VERITY
# define FS_IOC_ENABLE_VERITY    _IOW('f', 133, struct fsverity_enable_arg)
#endif

#endif
