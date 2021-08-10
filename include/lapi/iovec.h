// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_IOVEC_H__
#define LAPI_IOVEC_H__

#include "config.h"

#if !defined(HAVE_STRUCT_IOVEC)
struct iovec {
	void *iov_base;
	size_t iov_len;
};
#else
# include <sys/uio.h>
#endif

#endif /* LAPI_IOVEC_H__ */
