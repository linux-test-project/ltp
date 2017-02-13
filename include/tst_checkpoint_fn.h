/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
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

#ifndef TST_CHECKPOINT_FN__
#define TST_CHECKPOINT_FN__

/*
 * Checkpoint initializaton, must be done first.
 *
 * NOTE: tst_tmpdir() must be called beforehand.
 */
void tst_checkpoint_init(const char *file, const int lineno,
			 void (*cleanup_fn)(void));

/*
 * Waits for wakeup.
 *
 * @id: Checkpoint id, possitive number
 * @msec_timeout: Timeout in miliseconds, 0 == no timeout
 */
int tst_checkpoint_wait(unsigned int id, unsigned int msec_timeout);

/*
 * Wakes up sleeping process(es)/thread(s).
 *
 * @id: Checkpoint id, possitive number
 * @nr_wake: Number of processes/threads to wake up
 * @msec_timeout: Timeout in miliseconds, 0 == no timeout
 */
int tst_checkpoint_wake(unsigned int id, unsigned int nr_wake,
                        unsigned int msec_timeout);

void tst_safe_checkpoint_wait(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
			      unsigned int msec_timeout);

void tst_safe_checkpoint_wake(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
                              unsigned int nr_wake);

#endif /* TST_CHECKPOINT_FN__ */
