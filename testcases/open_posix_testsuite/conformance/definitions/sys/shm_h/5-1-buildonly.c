/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shmatt_t type is an unsigned integer that must be able to
 * store values at least as large than unsigned short.
 */

#include <sys/shm.h>
#include <limits.h>

/* The minimun acceptable value for USHRT_MAX */
#define MIN_USHRT_MAX 65535

shmatt_t tmin = 0;
shmatt_t tmax = USHRT_MAX;
shmatt_t tminmax = MIN_USHRT_MAX;
