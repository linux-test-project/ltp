/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the header defines the MS_SYNC flag option for msync().
 *
 * @pt:MF
 * @pt:SIO
 */

#include <sys/mman.h>

#ifndef MS_SYNC
#error MS_SYNC not defined
#endif
