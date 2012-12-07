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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ffsb_op.h"
#include "fileops.h"
#include "metaops.h"

ffsb_op_t ffsb_op_list[] = { {0, "read", ffsb_readfile, READ, fop_bench, NULL}
,
{1, "readall", ffsb_readall, READ, fop_bench, NULL}
,
{2, "write", ffsb_writefile, WRITE, fop_bench, NULL}
,
{3, "create", ffsb_createfile, WRITE, fop_bench, fop_age}
,
{4, "append", ffsb_appendfile, WRITE, fop_bench, fop_age}
,
{5, "delete", ffsb_deletefile, NA, fop_bench, fop_age}
,
{6, "metaop", ffsb_metaops, NA, metaops_metadir, NULL}
,
{7, "createdir", ffsb_createdir, NA, fop_bench, NULL}
,
{8, "stat", ffsb_stat, NA, fop_bench, NULL}
,
{9, "writeall", ffsb_writeall, WRITE, fop_bench, NULL}
,
{10, "writeall_fsync", ffsb_writeall_fsync, WRITE, fop_bench, NULL}
,
{11, "open_close", ffsb_open_close, NA, fop_bench, NULL}
,
{12, "write_fsync", ffsb_writefile_fsync, WRITE, fop_bench, NULL}
,
{13, "create_fsync", ffsb_createfile_fsync, WRITE, fop_bench, fop_age}
,
{14, "append_fsync", ffsb_appendfile_fsync, WRITE, fop_bench, fop_age}
,
};

void init_ffsb_op_results(ffsb_op_results_t * results)
{
	memset(results, 0, sizeof(ffsb_op_results_t));
}

static int exclusive_op(ffsb_op_results_t * results, unsigned int op_num)
{
	int i;
	int ret = 0;
	for (i = 0; i < FFSB_NUMOPS; i++) {
		if (i == op_num)
			continue;
		ret += results->ops[i];
	}

	if (ret)
		return 0;
	return 1;
}

static void generic_op_print(char *name, unsigned num, double op_pcnt,
			     double weigth_pcnt, double runtime, char *tput)
{
	printf("%20s : %12u\t%10.2lf\t%6.3lf%%\t\t%6.3lf%%\t  %11s\n",
	       name, num, num / runtime, op_pcnt, weigth_pcnt, tput);
}

static void print_op_results(unsigned int op_num, ffsb_op_results_t * results,
			     double runtime, unsigned total_ops,
			     unsigned total_weight)
{
	char buf[256];

	double op_pcnt = 100 * (double)results->ops[op_num] / (double)total_ops;
	double weight_pcnt = 100 * (double)results->op_weight[op_num] /
	    (double)total_weight;
	if (ffsb_op_list[op_num].throughput) {
		ffsb_printsize(buf, results->bytes[op_num] / runtime, 256);
		sprintf(buf, "%s/sec\0", buf);
	} else
		sprintf(buf, "NA\0");
	generic_op_print(ffsb_op_list[op_num].op_name, results->ops[op_num],
			 op_pcnt, weight_pcnt, runtime, buf);
}

#if 0
static void print_op_throughput(unsigned int op_num,
				ffsb_op_results_t * results, double runtime)
{
	if (ffsb_op_list[op_num].op_exl_print_fn != NULL)
		ffsb_op_list[op_num].op_exl_print_fn(results, runtime, op_num);
}
#endif

void print_results(struct ffsb_op_results *results, double runtime)
{
	int i;
	uint64_t total_ops = 0;
	uint64_t total_weight = 0;
	char buf[256];

	for (i = 0; i < FFSB_NUMOPS; i++) {
		total_ops += results->ops[i];
		total_weight += results->op_weight[i];
	}

	printf
	    ("             Op Name   Transactions\t Trans/sec\t% Trans\t    % Op Weight\t   Throughput\n");
	printf
	    ("             =======   ============\t =========\t=======\t    ===========\t   ==========\n");
	for (i = 0; i < FFSB_NUMOPS; i++)
		if (results->ops[i] != 0)
			print_op_results(i, results, runtime, total_ops,
					 total_weight);
	printf("-\n%.2lf Transactions per Second\n\n",
	       (double)total_ops / runtime);

	if (results->write_bytes || results->read_bytes)
		printf("Throughput Results\n===================\n");
	if (results->read_bytes) {
		ffsb_printsize(buf, results->read_bytes / runtime, 256);
		printf("Read Throughput: %s/sec\n", buf);
	}
	if (results->write_bytes) {
		ffsb_printsize(buf, results->write_bytes / runtime, 256);
		printf("Write Throughput: %s/sec\n", buf);
	}
}

char *op_get_name(int opnum)
{
	return ffsb_op_list[opnum].op_name;
}

void ops_setup_bench(ffsb_fs_t * fs)
{
	int i;
	for (i = 0; i < FFSB_NUMOPS; i++)
		ffsb_op_list[i].op_bench(fs, i);
}

void ops_setup_age(ffsb_fs_t * fs)
{
	int i;
	for (i = 0; i < FFSB_NUMOPS; i++)
		if (ffsb_op_list[i].op_age)
			ffsb_op_list[i].op_age(fs, i);
}

int ops_find_op(char *opname)
{
	int i;
	for (i = 0; i < FFSB_NUMOPS; i++)
		if (!strcmp(opname, ffsb_op_list[i].op_name))
			return i;
	return -1;
}

void add_results(struct ffsb_op_results *target, struct ffsb_op_results *src)
{
	int i;
	target->read_bytes += src->read_bytes;
	target->write_bytes += src->write_bytes;

	for (i = 0; i < FFSB_NUMOPS; i++) {
		target->ops[i] += src->ops[i];
		target->op_weight[i] += src->op_weight[i];
		target->bytes[i] += src->bytes[i];
	}
}

void do_op(struct ffsb_thread *ft, struct ffsb_fs *fs, unsigned op_num)
{
	ffsb_op_list[op_num].op_fn(ft, fs, op_num);
}
