/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2020-2023
 */

#ifndef TST_MINMAX_H__
#define TST_MINMAX_H__

#include <sys/param.h>

#ifndef MIN
# define MIN(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	(void) (&_a == &_b); \
	_a < _b ? _a : _b; \
})
#endif /* MIN */

#ifndef MAX
# define MAX(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	(void) (&_a == &_b); \
	_a > _b ? _a : _b; \
})
#endif /* MAX */

#endif	/* TST_MINMAX_H__ */
