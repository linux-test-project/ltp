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
#ifndef _RWLOCK_H
#define _RWLOCK_H

#include <pthread.h>

/* #define RWDEBUG 1 */

/* n_readers >= 0 means 0 or more readers */
/* n_readers < 0 means a writer */
struct rwlock {
	pthread_mutex_t plock;
	int n_readers;
	pthread_cond_t pcond;
#ifdef RWDEBUG
  int writer_tid;
  int n_read_waiting;
  int n_write_waiting;
#endif
};

void init_rwlock(struct rwlock *rw);

void rw_lock_read(struct rwlock *rw);
void rw_lock_write(struct rwlock *rw);

void rw_unlock_read(struct rwlock *rw);
void rw_unlock_write(struct rwlock *rw);

int rw_trylock_read(struct rwlock *rw);
int rw_trylock_write(struct rwlock *rw);



#endif
