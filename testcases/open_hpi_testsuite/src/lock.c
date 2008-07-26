/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2003, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 */
#include <oh_lock.h>

int oh_will_block = 0;
int lockcount = 0;

/* multi-threading support, use Posix mutex for data access */
/* initialize mutex used for data locking */
#include <glib/gthread.h>

GStaticRecMutex oh_main_lock = G_STATIC_REC_MUTEX_INIT;

int data_access_block_times(void)
{
        return(oh_will_block);
}

