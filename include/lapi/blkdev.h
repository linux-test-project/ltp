// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Linux Test Project
 *  Li Wang <liwang@redhat.com>
 */

#ifndef LAPI_BLKDEV_H__
#define LAPI_BLKDEV_H__

#ifdef HAVE_LINUX_BLKDEV_H
#include <linux/blkdev.h>
#endif

/* Define BLK_MAX_BLOCK_SIZE for older kernels */
#ifndef BLK_MAX_BLOCK_SIZE
#define BLK_MAX_BLOCK_SIZE 0x00010000 /* 64K */
#endif

#endif /* LAPI_BLKDEV_H */
