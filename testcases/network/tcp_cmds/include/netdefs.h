/*
 *   tcpcmds common definitions header (designed for to maximize modularity
 *   between IPv4 and IPv6 test code).
 *
 * Copyright (C) 2009, Cisco Systems Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef __NETDEFS_H
#define __NETDEFS_H

#include <netdb.h>

#if INET6

#include <netinet/ip6.h>
#include <netinet/icmp6.h>

typedef struct icmp6_hdr	icmp_t;
typedef struct sockaddr		sa_t;
typedef struct sockaddr_in6	sai_t;

#define AFI			AF_INET6
#define IERP			ICMP6_ECHO_REPLY
#define IERQ			ICMP6_ECHO_REQUEST
#define ICMP_PROTO		"ipv6-icmp"
#define PFI			PF_INET6

#else

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

typedef struct icmp		icmp_t;
typedef struct sockaddr		sa_t;
typedef struct sockaddr_in	sai_t;

#define AFI			AF_INET
#define IERP			ICMP_ECHOREPLY
#define IERQ			ICMP_ECHO
#define ICMP_PROTO		"icmp"
#define PFI			PF_INET

#endif

#define LISTEN_BACKLOG	10

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif
