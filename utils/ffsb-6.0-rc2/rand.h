/*
 *   Copyright (c) International Business Machines Corp., 2001-2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef _MTINT_H_
#define _MTINT_H_

#include "config.h"

#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

/* A guess of how many random bytes (not bits) */
/* will be consumed per iteration */
/* This is multiplied by the iteration count */
/* to get the size of the array in init_random() */
#define AVG_ITR_RNDBTS 2

/* Set a cap on the size of the array, note this */
/* is multiplied by AVG_ITR_RNDBTS */
#define MAX_RANDBUF_SIZE (10 * 1024)

#define MIN_RANDBUF_SIZE 1024


typedef struct randdata {
	int size;
	uint8_t *mt; /* the array of random bits  */
	int mti; /* mti==N+1 means mt[N] is not initialized */

	/* fallback random source, lrand48_r() */
#ifdef HAVE_LRAND48_R
	struct drand48_data data;
#endif
} randdata_t;

uint32_t getrandom(randdata_t *rd, uint32_t mod);
uint64_t getllrandom(randdata_t *rd, uint64_t mod);

/* pass in thread-local state, and est. number of "uses" */
/* pass in 0 for size if size is unknown/not important */
void init_random(randdata_t *state, uint32_t size);
void destroy_random(randdata_t *rd);
void randcleanup(void);

#endif
