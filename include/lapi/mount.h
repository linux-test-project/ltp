/*
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __MOUNT_H__
#define __MOUNT_H__

#ifndef MS_STRICTATIME
#define MS_STRICTATIME  1 << 24
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
