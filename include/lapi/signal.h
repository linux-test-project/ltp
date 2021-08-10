// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Daniel Díaz <daniel.diaz@linaro.org>
 */

#ifndef LAPI_SIGNAL_H__
#define LAPI_SIGNAL_H__

#include <signal.h>

/*
 * Some libc implementations might differ in the definitions they include. This
 * covers those differences for all tests to successfully build.
 */

#ifndef __SIGRTMIN
# define __SIGRTMIN 32
#endif
#ifndef __SIGRTMAX
# define __SIGRTMAX (_NSIG - 1)
#endif

#endif /* LAPI_SIGNAL_H__ */
