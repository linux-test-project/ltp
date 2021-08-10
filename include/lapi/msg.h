// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

#ifndef LAPI_MSG_H__
#define LAPI_MSG_H__

#include <sys/msg.h>

#ifndef MSG_COPY
# define MSG_COPY  040000  /* copy (not remove) all queue messages */
#endif

#ifndef MSG_STAT_ANY
# define MSG_STAT_ANY 13
#endif

#endif /* LAPI_MSG_H__ */
