/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
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
* along with this program.
*/

#ifndef __LAPI_SOCKET_H__
#define __LAPI_SOCKET_H__

#include <sys/socket.h>

#ifndef MSG_FASTOPEN
# define MSG_FASTOPEN	0x20000000 /* Send data in TCP SYN */
#endif

#ifndef SO_BUSY_POLL
# define SO_BUSY_POLL	46
#endif

#ifndef SOCK_DCCP
# define SOCK_DCCP		6
#endif

#ifndef SOCK_CLOEXEC
# define SOCK_CLOEXEC 02000000
#endif

#ifndef SOL_DCCP
# define SOL_DCCP		269
#endif

#endif /* __LAPI_SOCKET_H__ */
