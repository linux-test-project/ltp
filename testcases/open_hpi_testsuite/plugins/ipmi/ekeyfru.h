
/*      -*- linux-c -*-
 *
 * Copyright (c) 2006 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Vadim Revyakin <vadim.a.revyakin@intel.com>
 */
 
#ifndef _EKEYFRU_H_

#define  _EKEYFRU_H_
 
#define	CHANNEL_NUM(bytes) ((bytes)[0] & 0x3F)
#define	LINK_GROUPING_ID(bytes) ((bytes)[3])
#define	LINK_TYPE_EXTENSION(bytes) ((bytes[2] & 0xf0) >> 4)
#define	LINK_TYPE(bytes) (((bytes[2] & 0x0f) << 4) | (((bytes)[1] & 0xf0) >> 4))
#define	INTERFACE_TYPE(bytes) (((bytes)[0] & 0xc0) >> 6)
#define	PORTS(bytes) ((bytes)[1] & 0x0f)

#define DEBUG_EKEY 0

#endif

