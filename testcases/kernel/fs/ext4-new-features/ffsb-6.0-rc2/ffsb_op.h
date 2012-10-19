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
#ifndef _FFSB_OP_H_
#define _FFSB_OP_H_

#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

struct ffsb_op_results;
struct ffsb_thread;
struct ffsb_fs;

#define NA 0x00
#define READ 0x01
#define WRITE 0x02

/* This file handles all of the operations FFSB supports.  It has
 * tight interactions with the filesystem objects, but is otherwise
 * pretty abstract.
 */

/* The op callback */
typedef void (*ffsb_op_fn)(struct ffsb_thread *, struct ffsb_fs *,
			     unsigned op_num);

/* Operation results printing function */
typedef void (*ffsb_op_print_fn)(struct ffsb_op_results *, double secs,
				  unsigned int op_num);

/* Operation specific initialization for a filesystem */
typedef void (*ffsb_op_fs_fn)(struct ffsb_fs *, unsigned opnum);

typedef struct ffsb_op {
	unsigned int op_id;
	char *op_name;
	ffsb_op_fn op_fn;

	unsigned int throughput;
	/* The point of these two fields is to determine which set of
	 * files are being worked on.  Currently either data, meta, or
	 * aging.  Data and meta are mutually exclusive, so we only
	 * need two funcs.
	 */
	ffsb_op_fs_fn op_bench;
	ffsb_op_fs_fn op_age;
} ffsb_op_t;

/* List of all operations, located in ffsb_op.c */
extern ffsb_op_t ffsb_op_list[];

/* This *must* be updated when a new operation is added or one is
 * removed several other structures use it for statically sized arrays
 */
#define FFSB_NUMOPS (15)

/* Returns index of an op.
 * Returns -1 if opname isn't found, and its case sensitive :)
 */
int ops_find_op(char *opname);

typedef struct ffsb_op_results {
	/* Count of how many times each op was run */
	unsigned int ops[FFSB_NUMOPS];
	unsigned int op_weight[FFSB_NUMOPS];
	uint64_t bytes[FFSB_NUMOPS];

	uint64_t read_bytes;
	uint64_t write_bytes;
} ffsb_op_results_t;

void init_ffsb_op_results(struct ffsb_op_results *);
void print_results(struct ffsb_op_results *results, double runtime);
char *op_get_name(int opnum);

/* Setup the ops for the benchmark */
void ops_setup_bench(struct ffsb_fs *fs);

/* setup the ops for aging the filesystem */
void ops_setup_age(struct ffsb_fs *fs);

void add_results(struct ffsb_op_results *target, struct ffsb_op_results *src);

/* Run this op, using this thread state, on this filesystem */
void do_op(struct ffsb_thread *ft, struct ffsb_fs *fs, unsigned op_num);

#endif /* _FFSB_OP_H_ */
