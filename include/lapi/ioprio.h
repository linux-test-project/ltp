/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 */

#ifndef LAPI_IOPRIO_H__
#define LAPI_IOPRIO_H__

#include "config.h"

#ifdef HAVE_LINUX_IOPRIO_H
# include <linux/ioprio.h>
#else

enum {
	IOPRIO_CLASS_NONE = 0,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE,
};

enum {
	IOPRIO_WHO_PROCESS = 1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER,
};

# define IOPRIO_CLASS_SHIFT	(13)
# define IOPRIO_PRIO_MASK	((1UL << IOPRIO_CLASS_SHIFT) - 1)

# define IOPRIO_PRIO_CLASS(data)	((data) >> IOPRIO_CLASS_SHIFT)
# define IOPRIO_PRIO_VALUE(class, data)	(((class) << IOPRIO_CLASS_SHIFT) | data)

#endif

/* The RT and BE I/O priority classes have 8 priority levels 0..7 */
#ifdef IOPRIO_NR_LEVELS
# define IOPRIO_PRIO_NUM		IOPRIO_NR_LEVELS
#else
# define IOPRIO_PRIO_NUM		8
#endif

#ifndef IOPRIO_PRIO_LEVEL
# define IOPRIO_PRIO_LEVEL(data)	((data) & IOPRIO_PRIO_MASK)
#endif

#endif /* LAPI_IOPRIO_H__ */
