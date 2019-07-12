// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef LAPI_SECUREBITS_H
#define LAPI_SECUREBITS_H
# ifdef HAVE_LINUX_SECUREBITS_H
#  include <linux/securebits.h>
# else
#  define SECBIT_NO_CAP_AMBIENT_RAISE  6
# endif /* HAVE_LINUX_SECUREBITS_H*/
#endif /* LAPI_SECUREBITS_H */
