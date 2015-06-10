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
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <assert.h>

#include "config.h"

#include "ffsb.h"
#include "util.h"
#include "parser.h"

void init_ffsb_config(ffsb_config_t * fc, unsigned num_fs, unsigned num_tg)
{
	memset(fc, 0, sizeof(ffsb_config_t));

	fc->num_totalthreads = -1;
	fc->num_threadgroups = num_tg;
	fc->num_filesys = num_fs;

	fc->groups = ffsb_malloc(sizeof(ffsb_tg_t) * num_tg);
	fc->filesystems = ffsb_malloc(sizeof(ffsb_fs_t) * num_fs);
}

void init_ffsb_config_1fs(ffsb_config_t * fc, ffsb_fs_t * fs, ffsb_tg_t * tg)
{
	memset(fc, 0, sizeof(*fc));

	fc->num_totalthreads = tg_get_numthreads(tg);
	fc->num_threadgroups = 1;
	fc->num_filesys = 1;

	fc->groups = tg;
	fc->filesystems = fs;
}

void destroy_ffsb_config(ffsb_config_t * fc)
{
	int i;
	for (i = 0; i < fc->num_filesys; i++)
		destroy_ffsb_fs(&fc->filesystems[i]);

	for (i = 0; i < fc->num_threadgroups; i++)
		destroy_ffsb_tg(&fc->groups[i]);

	free(fc->groups);
	free(fc->filesystems);
}

void fc_set_time(ffsb_config_t * fc, unsigned time)
{
	fc->time = time;
}

unsigned fc_get_num_filesys(ffsb_config_t * fc)
{
	return fc->num_filesys;
}

struct ffsb_tg *fc_get_tg(ffsb_config_t * fc, unsigned num)
{
	assert(num < fc->num_threadgroups);
	return &fc->groups[num];
}

struct ffsb_fs *fc_get_fs(ffsb_config_t * fc, unsigned num)
{
	assert(num < fc->num_filesys);
	return &fc->filesystems[num];
}

void fc_set_num_totalthreads(ffsb_config_t * fc, int num)
{
	assert(num > 0);
	fc->num_totalthreads = num;
}

void fc_set_callout(ffsb_config_t * fc, char *callout)
{
	free(fc->callout);
	fc->callout = ffsb_strdup(callout);
}

char *fc_get_callout(ffsb_config_t * fc)
{
	return fc->callout;
}
