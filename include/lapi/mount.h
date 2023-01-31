// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2015-2022
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 */

#ifndef LAPI_MOUNT_H__
#define LAPI_MOUNT_H__

#include <sys/mount.h>

#ifndef MS_REC
# define MS_REC 16384
#endif

#ifndef MS_PRIVATE
# define MS_PRIVATE (1<<18)
#endif

#ifndef MS_STRICTATIME
# define MS_STRICTATIME (1 << 24)
#endif

#ifndef MNT_DETACH
# define MNT_DETACH 2
#endif

#ifndef MNT_EXPIRE
# define MNT_EXPIRE 4
#endif

#ifndef UMOUNT_NOFOLLOW
# define UMOUNT_NOFOLLOW 8
#endif

#ifndef MS_NOSYMFOLLOW
# define MS_NOSYMFOLLOW 256
#endif

#endif /* LAPI_MOUNT_H__ */
