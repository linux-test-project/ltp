/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
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

#include "ipmi.h"
void
posix_vlog(char *format, enum ipmi_log_type_e log_type, va_list ap)
{
     int do_nl = 1;
     char *msg = getenv("OHOI_TRACE_MSG");
     char *mem = getenv("OHOI_DBG_MEM");
     int do_debug = (getenv("OPENHPI_ERROR") && !strcmp("YES", getenv("OPENHPI_ERROR")));
     
     if ((msg || mem) && trace_msg_file) {
	vfprintf(trace_msg_file, format, ap);
	if ((log_type == IPMI_LOG_DEBUG_END) && do_nl)
		fprintf(trace_msg_file, "\n");
	if (mem) fprintf(trace_msg_file, "\n");
	fflush(trace_msg_file);
     }
     
	if (!do_debug) {
		return;
	}
    switch(log_type)
    {
	case IPMI_LOG_INFO:
	    printf("INFO: ");
	    do_nl = 1;
	    break;

	case IPMI_LOG_WARNING:
	    printf("WARN: ");
	    do_nl = 1;
	    break;

	case IPMI_LOG_SEVERE:
	    printf("SEVR: ");
	    do_nl = 1;
	    break;

	case IPMI_LOG_FATAL:
	    printf("FATL: ");
	    do_nl = 1;
	    break;

	case IPMI_LOG_ERR_INFO:
	    printf("EINF: ");
	    do_nl = 1;
	    break;

	case IPMI_LOG_DEBUG_START:
	    /* FALLTHROUGH */
	case IPMI_LOG_DEBUG:
	    printf("DEBG: ");
	    break;

	case IPMI_LOG_DEBUG_CONT:
	    /* FALLTHROUGH */
	case IPMI_LOG_DEBUG_END:
	    break;
    }

    vprintf(format, ap);

    if (do_nl)
	printf("\n");
}
