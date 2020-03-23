/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_MINMAX_H__
#define TST_MINMAX_H__

#ifndef MIN
# define MIN(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a < _b ? _a : _b; \
})
#endif /* MIN */

#ifndef MAX
# define MAX(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a > _b ? _a : _b; \
})
#endif /* MAX */

#endif	/* TST_MINMAX_H__ */
