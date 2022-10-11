// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef LAPI_PRCTL_H__
#define LAPI_PRCTL_H__

#include <sys/prctl.h>

#ifndef PR_SET_NAME
# define PR_SET_NAME 15
# define PR_GET_NAME 16
#endif

#ifndef PR_SET_SECCOMP
# define PR_GET_SECCOMP  21
# define PR_SET_SECCOMP  22
#endif

#ifndef PR_SET_TSC
# define PR_GET_TSC 25
# define PR_SET_TSC 26
# define PR_TSC_ENABLE  1
# define PR_TSC_SIGSEGV 2
#endif

#ifndef PR_SET_TIMERSLACK
# define PR_SET_TIMERSLACK 29
# define PR_GET_TIMERSLACK 30
#endif

#ifndef PR_SET_CHILD_SUBREAPER
# define PR_SET_CHILD_SUBREAPER	36
# define PR_GET_CHILD_SUBREAPER	37
#endif

#ifndef PR_SET_NO_NEW_PRIVS
# define PR_SET_NO_NEW_PRIVS 38
# define PR_GET_NO_NEW_PRIVS 39
#endif

#ifndef PR_SET_THP_DISABLE
# define PR_SET_THP_DISABLE 41
# define PR_GET_THP_DISABLE 42
#endif

#ifndef PR_CAP_AMBIENT
# define PR_CAP_AMBIENT             47
# define PR_CAP_AMBIENT_IS_SET      1
# define PR_CAP_AMBIENT_RAISE       2
# define PR_CAP_AMBIENT_LOWER       3
# define PR_CAP_AMBIENT_CLEAR_ALL   4
#endif

#ifndef PR_GET_SPECULATION_CTRL
# define PR_GET_SPECULATION_CTRL 52
# define PR_SET_SPECULATION_CTRL 53
#endif

#endif /* LAPI_PRCTL_H__ */
