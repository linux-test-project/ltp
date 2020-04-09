// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef LAPI_LOOP_H
#define LAPI_LOOP_H

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

#endif
