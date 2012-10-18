/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the header defines the MCL_FUTURE symbolic constant for
 * mlockall().
 *
 * @pt:ML
 */

#include <sys/mman.h>

#ifndef MCL_FUTURE
#error MCL_FUTURE not defined
#endif
