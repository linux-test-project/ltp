// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef LAPI_SECUREBITS_H__
#define LAPI_SECUREBITS_H__

# ifdef HAVE_LINUX_SECUREBITS_H
#  include <linux/securebits.h>
# endif

# ifndef SECBIT_NO_CAP_AMBIENT_RAISE
#  define SECBIT_NO_CAP_AMBIENT_RAISE  6
# endif

#endif /* LAPI_SECUREBITS_H__ */
