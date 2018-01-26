/*
 * Copyright (c) 2018 Oracle and/or its affiliates.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAPI_UDP_H__
#define LAPI_UDP_H__

#include <netinet/udp.h>

#ifndef UDPLITE_SEND_CSCOV
# define UDPLITE_SEND_CSCOV   10 /* sender partial coverage (as sent) */
#endif
#ifndef UDPLITE_RECV_CSCOV
# define UDPLITE_RECV_CSCOV   11 /* receiver partial coverage (threshold ) */
#endif

#endif	/* LAPI_UDP_H__ */
