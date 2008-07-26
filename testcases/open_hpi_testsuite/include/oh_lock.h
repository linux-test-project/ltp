/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2003, 2006
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
 *     Renier Morales <renier@openhpi.org>
 */

#ifndef __OH_LOCK_H
#define __OH_LOCK_H

#include <config.h>

extern int oh_will_block;

int data_access_block_times(void);

#ifdef OH_DBG_MSGS
/*
#define dbg_lock(format, ...) \
        do { \
                if (oh_get_global_bool(OPENHPI_DEBUG_LOCK)) { \
                        fprintf(stderr, "        LOCK: %s:%d:%s: ", __FILE__, __LINE__, __func__); \
                        fprintf(stderr, format "\n", ## __VA_ARGS__); \
                } \
        } while(0)
*/
#define dbg_lock(format, ...) \
        do { \
                if (getenv("OPENHPI_DEBUG_LOCK") && !strcmp("YES",getenv("OPENHPI_DEBUG_LOCK"))) { \
                        fprintf(stderr, "        LOCK: %s:%d:%s: ", __FILE__, __LINE__, __func__); \
                        fprintf(stderr, format "\n", ## __VA_ARGS__); \
                } \
        } while(0)
#else
#define dbg_lock(format, ...)
#endif
		 
/* multi-threading support, use Posix mutex for data access */
/* initialize mutex used for data locking */
#include <glib.h>
extern GStaticRecMutex oh_main_lock;
extern int lockcount;

#define data_access_lock_init()

#define data_access_lock()                                              \
        do {                                                            \
                dbg_lock("%p - Attempting lock",  g_thread_self());     \
                if (!g_static_rec_mutex_trylock(&oh_main_lock)) {       \
                        dbg_lock("%p - Lockcount: %d",  g_thread_self(), lockcount); \
                        dbg_lock("%p - Going to block for a lock now",  g_thread_self()); \
                        oh_will_block++;                                \
                        g_static_rec_mutex_lock(&oh_main_lock);         \
                        dbg_lock("%p - Got the lock after blocking",  g_thread_self()); \
                        lockcount++;                                    \
                } else {                                                \
                        dbg_lock("%p - Got the lock because no one had it",  g_thread_self()); \
                        lockcount++;                                    \
                        dbg_lock("%p - Lockcount: %d", g_thread_self(), lockcount); \
                }                                                       \
        } while(0)

#define data_access_unlock()                              \
        do {                                              \
                lockcount--;                              \
                g_static_rec_mutex_unlock(&oh_main_lock); \
                dbg_lock("%p - released the lock",  g_thread_self());      \
        } while(0)

#endif /* __OH_LOCK_H */
