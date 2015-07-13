/*
 * Copyright (c) 2015 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GETRANDOM_H__
#define __GETRANDOM_H__

#include "config.h"

#if HAVE_LINUX_RANDOM_H
#include <linux/random.h>
#endif

/*
 * Flags for getrandom(2)
 *
 * GRND_NONBLOCK	Don't block and return EAGAIN instead
 * GRND_RANDOM		Use the /dev/random pool instead of /dev/urandom
 */

#ifndef GRND_NONBLOCK
# define GRND_NONBLOCK	0x0001
#endif

#ifndef GRND_RANDOM
# define GRND_RANDOM	0x0002
#endif

#endif /* __GETRANDOM_H__ */
