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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

#include "config.h"
#include "rand.h"
#include "util.h"

#define RANDSRC "/dev/urandom"

static int randfd = -1;

/* close the file after we're done with the benchmark */
void randcleanup(void)
{
	if (randfd > 0)
		close(randfd);
}

/* We fill up the array with random bits from RANDSRC here and set index */
/* to 0 */
/* pre: state->size must be set and state->mt must be allocated! */
static void sgenrand(randdata_t * state)
{
	int got = 0;
	got = read(randfd, state->mt, state->size);
	if (got != state->size) {
		int i;
		/* fall back on lrand48 */
		/* printf("fallback_rand\n"); */

		for (i = got; i < state->size; i += 4) {
			long int rand = 0;
#ifdef HAVE_LRAND48
			lrand48_r(&(state->data), &rand);
#else
			rand = random();
#endif
			assert(rand != 0);
			state->mt[i] = (rand >> 24) & (512 - 1);
			state->mt[i + 1] = (rand >> 16) & (512 - 1);
			state->mt[i + 2] = (rand >> 8) & (512 - 1);
			state->mt[i + 3] = (rand) & (512 - 1);
		}

	}
	state->mti = 0;
}

/* returns 8 random bits */
static uint8_t genrand8(randdata_t * state)
{
	unsigned long ret = 0;
	if (state->mti >= state->size) {
/*		sgenrand(state); */
		state->mti = 0;
	}
	ret = state->mt[state->mti];
	state->mti++;
	return ret;
}

/* returns 32 random bits */
static uint32_t genrand32(randdata_t * state)
{
	uint8_t bytes[4];
	uint32_t ret = 0;

	bytes[0] = genrand8(state);
	bytes[1] = genrand8(state);
	bytes[2] = genrand8(state);
	bytes[3] = genrand8(state);

	ret = *((uint32_t *) bytes);	/* !!! hack */
	return ret;
}

void init_random(randdata_t * state, uint32_t iter)
{
	struct timeval time;
	if (iter == 0)
		state->size = MIN_RANDBUF_SIZE * AVG_ITR_RNDBTS;
	else if (iter > MAX_RANDBUF_SIZE)
		state->size = MAX_RANDBUF_SIZE * AVG_ITR_RNDBTS;
	else
		state->size = iter * AVG_ITR_RNDBTS;

	state->mt = ffsb_malloc(state->size);

	/* !!!! racy? add pthread_once stuff later  */
	if ((randfd < 0) && (randfd = open(RANDSRC, O_RDONLY)) < 0) {
		perror("open " RANDSRC);
		exit(1);
	}
	sgenrand(state);
	gettimeofday(&time, NULL);
#ifdef HAVE_LRAND48
	srand48_r(time.tv_sec, &state->data);
#endif
}

void destroy_random(randdata_t * rd)
{
	free(rd->mt);
}

/*
 * I've taken the liberty of slightly redesigning this stuff.
 * Instead of simply getting the full word of random bits
 * and throwing away most of it using the mod operator,
 * we should only get byte-sized chunks of random bits and
 * construct our random number that way with less wasteage - SR
 */
uint32_t getrandom(randdata_t * state, uint32_t mod)
{

	uint8_t bytes[4] = { 0, 0, 0, 0 };
	uint32_t ret;
	int num_bytes = 4;
	int i;

	if ((mod == 0) || (mod == 1))
		return 0;

	if (!(mod >> 8))
		num_bytes = 1;
	else if (!(mod >> 16))
		num_bytes = 2;
	else if (!(mod >> 24))
		num_bytes = 3;

	for (i = 0; i < num_bytes; i++)
		bytes[i] = genrand8(state);

	ret = (bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0];

	return ret % mod;
}

uint64_t getllrandom(randdata_t * state, uint64_t mod)
{
	uint64_t result = 0;
	uint64_t high = 0;
	uint32_t low = 0;

	if (mod == 0)
		return 0;

	/* ULONG_MAX comes from limits.h */
	if (mod < ULONG_MAX)
		return (uint64_t) getrandom(state, (uint32_t) mod);

	high = genrand32(state);

	low = genrand32(state);

	result = high << 32;
	result |= (uint64_t) low;

	assert(result != 0);
	assert(result > 0);

	return result % mod;
}
