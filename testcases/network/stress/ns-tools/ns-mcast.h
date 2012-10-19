/******************************************************************************/
/*                                                                            */
/*   Copyright (c) International Business Machines  Corp., 2006               */
/*                                                                            */
/*   This program is free software;  you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by     */
/*   the Free Software Foundation; either version 2 of the License, or        */
/*   (at your option) any later version.                                      */
/*                                                                            */
/*   This program is distributed in the hope that it will be useful,          */
/*   but WITHOUT ANY WARRANTY;  without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                */
/*   the GNU General Public License for more details.                         */
/*                                                                            */
/*   You should have received a copy of the GNU General Public License        */
/*   along with this program;  if not, write to the Free Software             */
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA  */
/*                                                                            */
/******************************************************************************/


/*
 * File:
 *	ns-mcast.h
 *
 * Description:
 *	Header file for multicast test.
 *	This file specifies structures and macors if missing
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Apr 21 2006 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

#ifndef _NS_MCAST_H
#define _NS_MCAST_H 1

#include <netinet/in.h>
#include <endian.h>


#ifndef MLD_LISTENER_QUERY
#  define MLD_LISTENER_QUERY          130
#endif


/* group_req */
#ifndef MCAST_JOIN_GROUP
# define MCAST_JOIN_GROUP		42
# define MCAST_BLOCK_SOURCE		43
# define MCAST_UNBLOCK_SOURCE		44
# define MCAST_LEAVE_GROUP		45

struct group_req
{
    uint32_t gr_interface;
    struct sockaddr_storage gr_group;
};

#endif	/* MCAST_JOIN_GROUP */


/* group_filter */
#ifndef MCAST_MSFILTER
# define MCAST_MSFILTER			48
# define MCAST_EXCLUDE	0
# define MCAST_INCLUDE	1

struct group_filter
{
    uint32_t gf_interface;
    struct sockaddr_storage gf_group;
    uint32_t gf_fmode;
    uint32_t gf_numsrc;
    struct sockaddr_storage gf_slist[1];
};

#define GROUP_FILTER_SIZE(numsrc) \
	(sizeof(struct group_filter) - sizeof(struct sockaddr_storage) \
	+ (numsrc) * sizeof(struct sockaddr_storage))

#endif	/* MCAST_MSFILTER */

#ifndef IGMP_ALL_HOSTS
# define IGMP_ALL_HOSTS		htonl(0xE0000001L)
#endif

#ifndef IGMPV3_HOST_MEMBERSHIP_REPORT
struct igmpv3_query {
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint32_t group;
# if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t qrv:3;
    uint8_t suppress:1;
    uint8_t resv:4;
# elif __BYTE_ORDER == __BIG_ENDIAN
     uint8_t resv:4;
     uint8_t suppress:1;
     uint8_t qrv:3;
# else
#  error "Failed to detect endian"
#endif
    uint8_t qqic;
    uint16_t nsrcs;
    uint32_t srcs[0];
};
#endif	/* IGMPV3_HOST_MEMBERSHIP_REPORT */

#endif	/* _NS_MCAST_H */
