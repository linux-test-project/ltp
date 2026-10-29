// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
*/

#ifndef LAPI_SOCKET_H__
#define LAPI_SOCKET_H__

#include "config.h"
#include <sys/socket.h>

#ifndef MSG_ZEROCOPY
# define MSG_ZEROCOPY	0x4000000 /* Use user data in kernel path */
#endif

#ifndef MSG_FASTOPEN
# define MSG_FASTOPEN	0x20000000 /* Send data in TCP SYN */
#endif

#ifndef SO_REUSEPORT
# define SO_REUSEPORT	15
#endif

#ifndef SO_BUSY_POLL
# define SO_BUSY_POLL	46
#endif

#ifndef SO_ATTACH_BPF
# define SO_ATTACH_BPF  50
#endif

#ifndef SO_ZEROCOPY
# define SO_ZEROCOPY	60
#endif

#ifndef SOCK_DCCP
# define SOCK_DCCP		6
#endif

#ifndef SOCK_CLOEXEC
# define SOCK_CLOEXEC 02000000
#endif

#ifndef AF_ALG
# define AF_ALG		38
#endif

#ifndef SOL_SCTP
# define SOL_SCTP	132
#endif

#ifndef SOL_UDPLITE
# define SOL_UDPLITE		136 /* UDP-Lite (RFC 3828) */
#endif

#ifndef SOL_DCCP
# define SOL_DCCP		269
#endif

#ifndef SOL_ALG
# define SOL_ALG		279
#endif

#ifndef SOL_TLS
# define SOL_TLS         282
#endif

#ifndef HAVE_STRUCT_MMSGHDR
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned int msg_len;
};
#endif

#endif /* LAPI_SOCKET_H__ */
