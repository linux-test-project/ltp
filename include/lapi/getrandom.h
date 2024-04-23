// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Linux Test Project
 */

#ifndef LAPI_GETRANDOM_H__
#define LAPI_GETRANDOM_H__

#include "config.h"

#ifdef HAVE_SYS_RANDOM_H
# include <sys/random.h>
#elif HAVE_LINUX_RANDOM_H
# include <linux/random.h>
#endif

#include "lapi/syscalls.h"

/*
 * Flags for getrandom(2)
 *
 * GRND_NONBLOCK	Don't block and return EAGAIN instead
 * GRND_RANDOM		Use the /dev/random pool instead of /dev/urandom
 */

#ifndef GRND_NONBLOCK
# define GRND_NONBLOCK	0x0001
#endif

#ifndef GRND_RANDOM
# define GRND_RANDOM	0x0002
#endif

#ifndef HAVE_SYS_RANDOM_H
static inline int getrandom(void *buf, size_t buflen, unsigned int flags)
{
	return tst_syscall(SYS_getrandom, buf, buflen, flags);
}
#endif

#endif /* LAPI_GETRANDOM_H__ */
