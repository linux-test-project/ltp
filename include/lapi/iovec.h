// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef IOVEC_H
#define IOVEC_H

#include "config.h"

#if !defined(HAVE_STRUCT_IOVEC)
struct iovec {
	void *iov_base;
	size_t iov_len;
};
#else
# include <sys/uio.h>
#endif

#endif /* IOVEC_H */
