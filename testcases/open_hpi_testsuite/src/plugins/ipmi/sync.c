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

SaErrorT ohoi_loop_until(loop_indicator_cb indicator, const void *cb_data, int to) 
{
	struct timeval tv1, tv2, tv3; 
        /* Wait 5 seconds for result */
        gettimeofday(&tv1, NULL);
	tv1.tv_sec += to;
	while (1) {

		if (indicator(cb_data))
			break;
		
		memset(&tv3, 0, sizeof(tv3));	
		gettimeofday(&tv2, NULL);
		if (tv2.tv_sec>tv1.tv_sec) 
			break;
		
		sel_select(ui_sel, NULL, 0, NULL, &tv3);
	}
	
	return ((indicator(cb_data))? SA_OK:SA_ERR_HPI_TIMEOUT);
}

static int simple_indicator(const void *cb_data)
{
        return (*(const int *)cb_data);
}

SaErrorT ohoi_loop(int *done)
{
        return (ohoi_loop_until(simple_indicator, done, 5));
}
