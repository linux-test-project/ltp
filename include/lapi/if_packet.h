// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 */

#ifndef LAPI_IF_PACKET_H__
#define LAPI_IF_PACKET_H__

#include "config.h"

#ifdef HAVE_LINUX_IF_PACKET_H
# include <linux/if_packet.h>
#endif

#ifndef PACKET_RX_RING
# define PACKET_RX_RING 5
#endif

#ifndef PACKET_VERSION
# define PACKET_VERSION 10
#endif

#ifndef PACKET_RESERVE
# define PACKET_RESERVE 12
#endif

#ifndef PACKET_VNET_HDR
# define PACKET_VNET_HDR 15
#endif

#ifndef PACKET_FANOUT
# define PACKET_FANOUT 18
#endif

#ifndef PACKET_FANOUT_ROLLOVER
# define PACKET_FANOUT_ROLLOVER 3
#endif

#ifndef HAVE_STRUCT_TPACKET_REQ3
# define TPACKET_V3 2

struct tpacket_req3 {
	unsigned int	tp_block_size;
	unsigned int	tp_block_nr;
	unsigned int	tp_frame_size;
	unsigned int	tp_frame_nr;
	unsigned int	tp_retire_blk_tov;
	unsigned int	tp_sizeof_priv;
	unsigned int	tp_feature_req_word;
};
#endif

#endif /* LAPI_IF_PACKET_H__ */
