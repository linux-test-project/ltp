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
#ifndef _PARSER_H_
#define _PARSER_H_

#include "ffsb.h"
#include "list.h"

#define COMMENT_CHAR	'#'

#define STORE_SINGLE		0x0001
#define STORE_LIST		0x0002

#define TYPE_U32		0x0001
#define	TYPE_U64		0x0002
#define TYPE_STRING		0x0004
#define TYPE_BOOLEAN		0x0008
#define TYPE_DOUBLE		0x0010
#define TYPE_RANGE		0x0020
#define TYPE_SIZEWEIGHT		0x0040
#define TYPE_DEPRECATED		0x0080
#define TYPE_WEIGHT		0x0100
#define TYPE_SIZE32		0x0200
#define TYPE_SIZE64		0x0400

#define ROOT			0x0001
#define THREAD_GROUP		0x0002
#define FILESYSTEM		0x0004
#define END			0x0008
#define STATS			0x0010

#define GLOBAL_OPTIONS {						\
	{"num_filesystems", NULL, TYPE_DEPRECATED, STORE_SINGLE},	\
	{"num_threadgroups", NULL, TYPE_DEPRECATED, STORE_SINGLE},	\
	{"verbose", NULL, TYPE_BOOLEAN, STORE_SINGLE},			\
	{"time", NULL, TYPE_U32, STORE_SINGLE},				\
	{"directio", NULL, TYPE_BOOLEAN, STORE_SINGLE},			\
	{"bufferio", NULL, TYPE_BOOLEAN, STORE_SINGLE},			\
	{"alignio", NULL, TYPE_BOOLEAN, STORE_SINGLE},			\
	{"callout", NULL, TYPE_STRING, STORE_SINGLE},			\
	{NULL, NULL, 0, 0} }

#define THREADGROUP_OPTIONS {						\
	{"bindfs", NULL, TYPE_STRING, STORE_SINGLE},			\
	{"num_threads", NULL, TYPE_U32, STORE_SINGLE},			\
	{"read_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"readall_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"read_random", NULL, TYPE_BOOLEAN, STORE_SINGLE},		\
	{"read_skip", NULL, TYPE_U32, STORE_SINGLE},			\
	{"read_size", NULL, TYPE_SIZE64, STORE_SINGLE},			\
	{"read_blocksize", NULL, TYPE_SIZE32, STORE_SINGLE},		\
	{"read_skipsize", NULL, TYPE_SIZE32, STORE_SINGLE},		\
	{"write_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"write_fsync_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},	\
	{"write_random", NULL, TYPE_BOOLEAN, STORE_SINGLE},		\
	{"fsync_file", NULL, TYPE_DEPRECATED, STORE_SINGLE},		\
	{"write_size", NULL, TYPE_SIZE64, STORE_SINGLE},		\
	{"write_blocksize", NULL, TYPE_SIZE32, STORE_SINGLE},		\
	{"create_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"create_fsync_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},	\
	{"delete_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"append_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"append_fsync_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},	\
	{"metaop_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"createdir_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"op_delay", NULL, TYPE_U32, STORE_SINGLE},			\
	{"stat_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"writeall_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{"writeall_fsync_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},	\
	{"open_close_weight", NULL, TYPE_WEIGHT, STORE_SINGLE},		\
	{NULL, NULL, 0} }

#define FILESYSTEM_OPTIONS {						\
	{"location", NULL, TYPE_STRING, STORE_SINGLE},			\
	{"num_files", NULL, TYPE_U32, STORE_SINGLE},			\
	{"num_dirs", NULL, TYPE_U32, STORE_SINGLE},			\
	{"reuse", NULL, TYPE_BOOLEAN, STORE_SINGLE},			\
	{"min_filesize", NULL, TYPE_SIZE64, STORE_SINGLE},		\
	{"max_filesize", NULL, TYPE_SIZE64, STORE_SINGLE},		\
	{"create_blocksize", NULL, TYPE_SIZE32, STORE_SINGLE},		\
	{"age_blocksize", NULL, TYPE_SIZE32, STORE_SINGLE},		\
	{"desired_util", NULL, TYPE_DOUBLE, STORE_SINGLE},		\
	{"agefs", NULL, TYPE_BOOLEAN, STORE_SINGLE},			\
	{"size_weight", NULL, TYPE_SIZEWEIGHT, STORE_LIST},		\
	{"init_util", NULL, TYPE_DOUBLE, STORE_SINGLE},			\
	{"init_size", NULL, TYPE_SIZE64, STORE_SINGLE},			\
	{"clone", NULL, TYPE_STRING, STORE_SINGLE},			\
	{NULL, NULL, 0} }

#define STATS_OPTIONS {							\
	{"enable_stats", NULL, TYPE_BOOLEAN, STORE_SINGLE},		\
	{"enable_range", NULL, TYPE_BOOLEAN, STORE_SINGLE},		\
	{"ignore", NULL, TYPE_STRING, STORE_LIST},			\
	{"msec_range", NULL, TYPE_RANGE, STORE_LIST},			\
	{NULL, NULL, 0} }

#define CONTAINER_DESC {				\
	{"filesystem", FILESYSTEM, 10},			\
	{"threadgroup", THREAD_GROUP, 11},		\
	{"end", END, 3},				\
	{"stats", STATS, 5},				\
	{NULL, 0, 0} }

typedef struct container {
	struct config_options *config;
	uint32_t type;
	struct container *child;
	struct container *next;
} container_t;

typedef struct config_options {
	char *name;
	void *value;
	int type;
	int storage_type;
} config_options_t;

typedef struct container_desc {
	char *name;
	uint16_t type;
	uint16_t size;
} container_desc_t;

typedef struct range {
	double a;
	double b;
} range_t;

typedef struct value_list {
	void *value;
	struct list_head list;
} value_list_t;

void ffsb_parse_newconfig(ffsb_config_t *fc, char *filename);

#endif
