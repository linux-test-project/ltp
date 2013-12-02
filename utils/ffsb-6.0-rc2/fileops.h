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
#ifndef _FILEOPS_H_
#define _FILEOPS_H_

#include "ffsb_thread.h"
#include "ffsb.h"
#include "ffsb_op.h"
#include "ffsb_fs.h"

void ffsb_readfile(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_readall(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_writefile(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_writefile_fsync(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_writeall(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_writeall_fsync(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_createfile(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_createfile_fsync(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_deletefile(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_appendfile(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_appendfile_fsync(ffsb_thread_t *tconfig, ffsb_fs_t *, unsigned opnum);
void ffsb_stat(ffsb_thread_t *ft, ffsb_fs_t *fs, unsigned opnum);
void ffsb_open_close(ffsb_thread_t *ft, ffsb_fs_t *fs, unsigned opnum);

struct ffsb_op_results;

void ffsb_read_print_exl(struct ffsb_op_results *, double secs, unsigned op_num);
void ffsb_write_print_exl(struct ffsb_op_results *, double secs, unsigned op_num);
void ffsb_create_print_exl(struct ffsb_op_results *, double secs, unsigned op_num);
void ffsb_append_print_exl(struct ffsb_op_results *, double secs, unsigned op_num);

/* Set up ops for either aging or benchmarking */
void fop_bench(ffsb_fs_t *fs, unsigned opnum);
void fop_age(ffsb_fs_t *fs, unsigned opnum);

#endif /* _FILEOPS_H_ */
