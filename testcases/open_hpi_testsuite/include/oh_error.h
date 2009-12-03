/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *
 */

#ifndef __OH_ERROR_H
#define __OH_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <config.h>

/* this is put here intentionally as there are too many instances
 * of unqualified sprintf calls in plugin code. Use snprintf instead
 * to ensure there are no buffer overruns 
 */
/* Unfortunately, sprintf is used in the system headers of some versions
 * of Solaris, so we can't poison sprintf there.
 */
#if !defined(__sun) || !defined(__SVR4)
#undef sprintf
#pragma GCC poison sprintf
#endif

#define OH_ERROR "OPENHPI_ERROR"
#define OH_DEBUG "OPENHPI_DEBUG"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OH_DBG_MSGS
#define err(format, ...) \
	do { \
		syslog(3, "ERROR: (%s, %d, "format")", __FILE__, __LINE__,## __VA_ARGS__); \
		if (getenv(OH_ERROR) && !strcmp("YES", getenv(OH_ERROR))) { \
			fprintf(stderr, "%s:%d ("format")\n", __FILE__, __LINE__, ## __VA_ARGS__); \
		} \
	} while(0)
#else
#define err(format, ...)
#endif

#ifdef OH_DBG_MSGS
#define warn(format, ...) \
	do { \
		syslog(3, "WARNING: (%s, %d, "format")", __FILE__, __LINE__,## __VA_ARGS__); \
		if (getenv(OH_ERROR) && !strcmp("YES", getenv(OH_ERROR))) { \
			fprintf(stderr, "%s:%d ("format")\n", __FILE__, __LINE__, ## __VA_ARGS__); \
		} \
	} while(0)
#else
#define warn(format, ...)
#endif

#ifdef OH_DBG_MSGS
#define dbg(format, ...) \
        do { \
                if (getenv(OH_DEBUG) && !strcmp("YES", getenv(OH_DEBUG))) { \
                        fprintf(stderr, " %s:%d:%s: ", __FILE__, __LINE__, __func__); \
                        fprintf(stderr, format "\n", ## __VA_ARGS__); \
                } \
        } while(0)
#else
#define dbg(format, ...)
#endif

#ifdef __cplusplus
}
#endif
        
#endif /* __OH_ERROR_H */

