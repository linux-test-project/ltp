/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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

#ifndef __LIBRPC_TIRPC_H__
#define __LIBRPC_TIRPC_H__

/*
 * Returns a network socket bound to an arbitrary port.
 * domain - AF_INET or AF_INET6,
 * type - SOCK_DGRAM, SOCK_STREAM
 * Returns -1 if failed (with set errno)
 */
int bound_socket(int domain, int type);

#endif /* __LIBRPC_TIRPC_H__ */
