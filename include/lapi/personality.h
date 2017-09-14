/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
