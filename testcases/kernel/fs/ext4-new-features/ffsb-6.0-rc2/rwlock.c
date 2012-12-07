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
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

/* #define RWDEBUG 1 */
#include "config.h"
#include "rwlock.h"

void init_rwlock(struct rwlock *rw)
{
	rw->n_readers = 0;
	pthread_mutex_init(&rw->plock, NULL);
	pthread_cond_init(&rw->pcond, NULL);
#ifdef RWDEBUG
	rw->n_write_waiting = 0;
	rw->n_read_waiting = 0;
	rw->writer_tid = -1;
#endif
}

void rw_lock_read(struct rwlock *rw)
{
	pthread_mutex_lock(&rw->plock);
#ifdef RWDEBUG
	rw->n_read_waiting++;
#endif
	while (rw->n_readers < 0)
		pthread_cond_wait(&rw->pcond, &rw->plock);
	rw->n_readers++;
#ifdef RWDEBUG
	rw->n_read_waiting--;
#endif
	pthread_mutex_unlock(&rw->plock);
}

void rw_lock_write(struct rwlock *rw)
{
	pthread_mutex_lock(&rw->plock);
#ifdef RWDEBUG
	rw->n_write_waiting++;
#endif
	while (rw->n_readers != 0)
		pthread_cond_wait(&rw->pcond, &rw->plock);
	rw->n_readers = -1;
#ifdef RWDEBUG
	rw->n_write_waiting--;
	rw->writer_tid = (int)pthread_self();
#endif
	pthread_mutex_unlock(&rw->plock);

}

void rw_unlock_read(struct rwlock *rw)
{
	pthread_mutex_lock(&rw->plock);
	rw->n_readers -= 1;
	pthread_cond_signal(&rw->pcond);
	pthread_mutex_unlock(&rw->plock);
}

void rw_unlock_write(struct rwlock *rw)
{
	pthread_mutex_lock(&rw->plock);
	rw->n_readers = 0;
	pthread_cond_broadcast(&rw->pcond);
	pthread_mutex_unlock(&rw->plock);
}

int rw_trylock_read(struct rwlock *rw)
{
	int ret = 1;
	pthread_mutex_lock(&rw->plock);
	if (rw->n_readers >= 0) {
		rw->n_readers++;
		ret = 0;
	}
	pthread_mutex_unlock(&rw->plock);
	return ret;
}

int rw_trylock_write(struct rwlock *rw)
{
	int ret = 1;
	pthread_mutex_lock(&rw->plock);
	if (rw->n_readers == 0) {
		ret = 0;
		rw->n_readers = -1;
	}
	pthread_mutex_unlock(&rw->plock);
	return ret;
}
