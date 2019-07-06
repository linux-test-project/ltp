// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 */

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

#ifndef _UTSNAME_LENGTH
# define _UTSNAME_LENGTH 65
#endif

#ifndef _UTSNAME_DOMAIN_LENGTH
# define _UTSNAME_DOMAIN_LENGTH _UTSNAME_LENGTH
#endif
