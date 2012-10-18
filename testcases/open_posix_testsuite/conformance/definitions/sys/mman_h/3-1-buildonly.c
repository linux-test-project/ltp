/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the header defines the MAP_SHARED flag option.
 *
 * @pt:MF
 * @pt:SHM
 */

#include <sys/mman.h>

#ifndef MAP_SHARED
#error MAP_SHARED not defined
#endif
