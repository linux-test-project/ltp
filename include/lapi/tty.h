// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_TTY_H__
#define LAPI_TTY_H__

#ifdef HAVE_LINUX_TTY_H
# include <linux/tty.h>
#endif

#ifndef N_HDLC
# define N_HDLC 13
#endif

#ifndef N_SLCAN
# define N_SLCAN		17	/* Serial / USB serial CAN Adaptors */
#endif

#endif /* LAPI_TTY_H__ */
