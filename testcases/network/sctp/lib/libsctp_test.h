/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * libsctp_test.h 
 *
 * HISTORY
 *	04/2002 Created by Mingqin Liu	
 *
 * RESTRICTIONS:
 *
 */

#ifndef LIBSCTP_TEST_H
#define LIBSCTP_TEST_H

#include <netinet/in.h>
#include <sys/ioctl.h>
#include <linux/if.h>

typedef struct v4_v6 {
        int     has_v4;
        char    v4_addr[64];
        int     has_v6;
        char    v6_addr[64];
        char    if_name[16];
} local_addr_t;


struct sockaddr_storage *
append_addr(const char *parm,
            struct sockaddr_storage *addrs, 
	    int *ret_count, 
	    int port);

void get_ip_addresses(local_addr_t *local_addrs, int * count);

int set_nonblock(int s);

#endif
