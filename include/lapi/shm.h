// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_SHM_H__
#define LAPI_SHM_H__

#include <limits.h>

#ifndef SHM_STAT_ANY
# define SHM_STAT_ANY 15
#endif

#ifndef SHMMIN
# define SHMMIN 1
#endif

#ifndef SHMMAX
# define SHMMAX (ULONG_MAX - (1UL << 24))
#endif

#ifndef SHMMNI
# define SHMMNI 4096
#endif

#endif /* LAPI_SHM_H__ */
