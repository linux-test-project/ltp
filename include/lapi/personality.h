// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 */

/* In the Linux kernel and glibc enums are (mostly) used for the constants,
 * but in musl macros are used.
 */

#ifndef PERSONALITY_H
#define PERSONALITY_H

#include <sys/personality.h>

#ifndef UNAME26
# define UNAME26 0x0020000
#endif

#ifndef READ_IMPLIES_EXEC
# define READ_IMPLIES_EXEC 0x0400000
#endif

#endif	/* PERSONALITY_H */
