// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 */

#ifndef __MOUNT_H__
#define __MOUNT_H__

#ifndef MS_REC
#define MS_REC 16384
#endif

#ifndef MS_PRIVATE
#define MS_PRIVATE (1<<18)
#endif

#ifndef MS_STRICTATIME
#define MS_STRICTATIME (1 << 24)
#endif

#ifndef MNT_DETACH
#define MNT_DETACH 2
#endif

#ifndef MNT_EXPIRE
#define MNT_EXPIRE 4
#endif

#ifndef UMOUNT_NOFOLLOW
#define UMOUNT_NOFOLLOW 8
#endif

#endif /* __MOUNT_H__ */
