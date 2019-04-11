// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

#ifndef LAPI_TIMEX_H__
# define LAPI_TIMEX_H__

#define ADJ_ALL (ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR |  \
		 ADJ_ESTERROR | ADJ_STATUS | ADJ_TIMECONST |  \
		 ADJ_TICK)

#ifndef ADJ_OFFSET_SS_READ
# define ADJ_OFFSET_SS_READ 0xa001
#endif

#ifndef ADJ_NANO
# define ADJ_NANO 0x2000
#endif

#ifndef STA_NANO
# define STA_NANO 0x2000
#endif

#ifndef ADJ_MICRO
# define ADJ_MICRO 0x1000
#endif

#endif/* LAPI_TIMEX_H__ */
