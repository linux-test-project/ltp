/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
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
 *
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 *
 */

/*
 * Contains common content for all swapon tests
 */

#ifndef __LIBSWAPON_H__
#define __LIBSWAPON_H__

/*
 * Make a swap file
 */
int make_swapfile(void (cleanup)(void), const char *swapfile, int safe);

/*
 * Check swapon/swapoff support status of filesystems or files
 * we are testing on.
 */
void is_swap_supported(void (cleanup)(void), const char *filename);
#endif /* __LIBSWAPON_H__ */
