// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2015-2022
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LAPI_MOUNT_H__
#define LAPI_MOUNT_H__

#include <stdint.h>
#include <sys/mount.h>
#include "config.h"

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

#ifndef HAVE_STRUCT_MNT_ID_REQ
struct mnt_id_req {
	uint32_t size;
	uint32_t spare;
	uint64_t mnt_id;
	uint64_t param;
};
#endif

#ifndef HAVE_STRUCT_STATMOUNT
struct statmount {
	uint32_t size;
	uint32_t __spare1;
	uint64_t mask;
	uint32_t sb_dev_major;
	uint32_t sb_dev_minor;
	uint64_t sb_magic;
	uint32_t sb_flags;
	uint32_t fs_type;
	uint64_t mnt_id;
	uint64_t mnt_parent_id;
	uint32_t mnt_id_old;
	uint32_t mnt_parent_id_old;
	uint64_t mnt_attr;
	uint64_t mnt_propagation;
	uint64_t mnt_peer_group;
	uint64_t mnt_master;
	uint64_t propagate_from;
	uint32_t mnt_root;
	uint32_t mnt_point;
	uint64_t __spare2[50];
	char str[];
};
#endif

#ifndef MNT_ID_REQ_SIZE_VER0
# define MNT_ID_REQ_SIZE_VER0 24
#endif

#ifndef STATMOUNT_SB_BASIC
# define STATMOUNT_SB_BASIC 0x00000001U
#endif

#ifndef STATMOUNT_MNT_BASIC
# define STATMOUNT_MNT_BASIC 0x00000002U
#endif

#ifndef STATMOUNT_PROPAGATE_FROM
# define STATMOUNT_PROPAGATE_FROM 0x00000004U
#endif

#ifndef STATMOUNT_MNT_ROOT
# define STATMOUNT_MNT_ROOT 0x00000008U
#endif

#ifndef STATMOUNT_MNT_POINT
# define STATMOUNT_MNT_POINT 0x00000010U
#endif

#ifndef STATMOUNT_FS_TYPE
# define STATMOUNT_FS_TYPE 0x00000020U
#endif

#ifndef LSMT_ROOT
# define LSMT_ROOT 0xffffffffffffffff
#endif

#endif /* LAPI_MOUNT_H__ */
