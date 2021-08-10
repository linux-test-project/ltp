// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_IOCTL_H__
#define LAPI_IOCTL_H__

#include "config.h"
#include <sys/ioctl.h>

/* musl not including it in <sys/ioctl.h> */
#include <sys/ttydefaults.h>

#ifndef TIOCVHANGUP
# define TIOCVHANGUP 0x5437
#endif

#ifndef HAVE_STRUCT_TERMIO
# ifndef NCC
#  ifdef __powerpc__
#   define NCC 10
#  else
#   define NCC 8
#  endif
# endif /* NCC */

struct termio
  {
    unsigned short int c_iflag;		/* input mode flags */
    unsigned short int c_oflag;		/* output mode flags */
    unsigned short int c_cflag;		/* control mode flags */
    unsigned short int c_lflag;		/* local mode flags */
    unsigned char c_line;		/* line discipline */
    unsigned char c_cc[NCC];		/* control characters */
};
#endif /* HAVE_STRUCT_TERMIO */

#endif /* LAPI_IOCTL_H__ */
