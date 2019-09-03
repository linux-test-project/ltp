// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * Essential Extended Berkeley Packet Filter (eBPF) headers
 *
 * Mostly copied/adapted from linux/bpf.h and libbpf so that we can perform
 * some eBPF testing without any external dependencies.
 */

#ifndef BPF_H
# define BPF_H

#include <stdint.h>

#include "lapi/syscalls.h"

/* Start copy from linux/bpf.h */
enum bpf_cmd {
	BPF_MAP_CREATE,
	BPF_MAP_LOOKUP_ELEM,
	BPF_MAP_UPDATE_ELEM,
	BPF_MAP_DELETE_ELEM,
	BPF_MAP_GET_NEXT_KEY,
	BPF_PROG_LOAD,
	BPF_OBJ_PIN,
	BPF_OBJ_GET,
	BPF_PROG_ATTACH,
	BPF_PROG_DETACH,
	BPF_PROG_TEST_RUN,
	BPF_PROG_GET_NEXT_ID,
	BPF_MAP_GET_NEXT_ID,
	BPF_PROG_GET_FD_BY_ID,
	BPF_MAP_GET_FD_BY_ID,
	BPF_OBJ_GET_INFO_BY_FD,
	BPF_PROG_QUERY,
	BPF_RAW_TRACEPOINT_OPEN,
	BPF_BTF_LOAD,
	BPF_BTF_GET_FD_BY_ID,
	BPF_TASK_FD_QUERY,
	BPF_MAP_LOOKUP_AND_DELETE_ELEM,
	BPF_MAP_FREEZE,
};

enum bpf_map_type {
	BPF_MAP_TYPE_UNSPEC,
	BPF_MAP_TYPE_HASH,
	BPF_MAP_TYPE_ARRAY,
	BPF_MAP_TYPE_PROG_ARRAY,
	BPF_MAP_TYPE_PERF_EVENT_ARRAY,
	BPF_MAP_TYPE_PERCPU_HASH,
	BPF_MAP_TYPE_PERCPU_ARRAY,
	BPF_MAP_TYPE_STACK_TRACE,
	BPF_MAP_TYPE_CGROUP_ARRAY,
	BPF_MAP_TYPE_LRU_HASH,
	BPF_MAP_TYPE_LRU_PERCPU_HASH,
	BPF_MAP_TYPE_LPM_TRIE,
	BPF_MAP_TYPE_ARRAY_OF_MAPS,
	BPF_MAP_TYPE_HASH_OF_MAPS,
	BPF_MAP_TYPE_DEVMAP,
	BPF_MAP_TYPE_SOCKMAP,
	BPF_MAP_TYPE_CPUMAP,
	BPF_MAP_TYPE_XSKMAP,
	BPF_MAP_TYPE_SOCKHASH,
	BPF_MAP_TYPE_CGROUP_STORAGE,
	BPF_MAP_TYPE_REUSEPORT_SOCKARRAY,
	BPF_MAP_TYPE_PERCPU_CGROUP_STORAGE,
	BPF_MAP_TYPE_QUEUE,
	BPF_MAP_TYPE_STACK,
	BPF_MAP_TYPE_SK_STORAGE,
};

#define BPF_OBJ_NAME_LEN 16U

#define BPF_ANY		0 /* create new element or update existing */
#define BPF_NOEXIST	1 /* create new element if it didn't exist */
#define BPF_EXIST	2 /* update existing element */
#define BPF_F_LOCK	4 /* spin_lock-ed map_lookup/map_update */

#define aligned_uint64_t uint64_t __attribute__((aligned(8)))

union bpf_attr {
	struct { /* anonymous struct used by BPF_MAP_CREATE command */
		uint32_t	map_type;	/* one of enum bpf_map_type */
		uint32_t	key_size;	/* size of key in bytes */
		uint32_t	value_size;	/* size of value in bytes */
		uint32_t	max_entries;	/* max number of entries in a map */
		uint32_t	map_flags;	/* BPF_MAP_CREATE related
					 * flags defined above.
					 */
		uint32_t	inner_map_fd;	/* fd pointing to the inner map */
		uint32_t	numa_node;	/* numa node (effective only if
					 * BPF_F_NUMA_NODE is set).
					 */
		char	map_name[BPF_OBJ_NAME_LEN];
		uint32_t	map_ifindex;	/* ifindex of netdev to create on */
		uint32_t	btf_fd;		/* fd pointing to a BTF type data */
		uint32_t	btf_key_type_id;	/* BTF type_id of the key */
		uint32_t	btf_value_type_id;	/* BTF type_id of the value */
	};

	struct { /* anonymous struct used by BPF_MAP_*_ELEM commands */
		uint32_t		map_fd;
		aligned_uint64_t	key;
		union {
			aligned_uint64_t value;
			aligned_uint64_t next_key;
		};
		uint64_t		flags;
	};

	struct { /* anonymous struct used by BPF_PROG_LOAD command */
		uint32_t		prog_type;	/* one of enum bpf_prog_type */
		uint32_t		insn_cnt;
		aligned_uint64_t	insns;
		aligned_uint64_t	license;
		uint32_t		log_level;	/* verbosity level of verifier */
		uint32_t		log_size;	/* size of user buffer */
		aligned_uint64_t	log_buf;	/* user supplied buffer */
		uint32_t		kern_version;	/* not used */
		uint32_t		prog_flags;
		char		prog_name[BPF_OBJ_NAME_LEN];
		uint32_t		prog_ifindex;	/* ifindex of netdev to prep for */
		/* For some prog types expected attach type must be known at
		 * load time to verify attach type specific parts of prog
		 * (context accesses, allowed helpers, etc).
		 */
		uint32_t		expected_attach_type;
		uint32_t		prog_btf_fd;	/* fd pointing to BTF type data */
		uint32_t		func_info_rec_size;	/* userspace bpf_func_info size */
		aligned_uint64_t	func_info;	/* func info */
		uint32_t		func_info_cnt;	/* number of bpf_func_info records */
		uint32_t		line_info_rec_size;	/* userspace bpf_line_info size */
		aligned_uint64_t	line_info;	/* line info */
		uint32_t		line_info_cnt;	/* number of bpf_line_info records */
	};

	struct { /* anonymous struct used by BPF_OBJ_* commands */
		aligned_uint64_t	pathname;
		uint32_t		bpf_fd;
		uint32_t		file_flags;
	};

	struct { /* anonymous struct used by BPF_PROG_ATTACH/DETACH commands */
		uint32_t		target_fd;	/* container object to attach to */
		uint32_t		attach_bpf_fd;	/* eBPF program to attach */
		uint32_t		attach_type;
		uint32_t		attach_flags;
	};

	struct { /* anonymous struct used by BPF_PROG_TEST_RUN command */
		uint32_t		prog_fd;
		uint32_t		retval;
		uint32_t		data_size_in;	/* input: len of data_in */
		uint32_t		data_size_out;	/* input/output: len of data_out
						 *   returns ENOSPC if data_out
						 *   is too small.
						 */
		aligned_uint64_t	data_in;
		aligned_uint64_t	data_out;
		uint32_t		repeat;
		uint32_t		duration;
		uint32_t		ctx_size_in;	/* input: len of ctx_in */
		uint32_t		ctx_size_out;	/* input/output: len of ctx_out
						 *   returns ENOSPC if ctx_out
						 *   is too small.
						 */
		aligned_uint64_t	ctx_in;
		aligned_uint64_t	ctx_out;
	} test;

	struct { /* anonymous struct used by BPF_*_GET_*_ID */
		union {
			uint32_t		start_id;
			uint32_t		prog_id;
			uint32_t		map_id;
			uint32_t		btf_id;
		};
		uint32_t		next_id;
		uint32_t		open_flags;
	};

	struct { /* anonymous struct used by BPF_OBJ_GET_INFO_BY_FD */
		uint32_t		bpf_fd;
		uint32_t		info_len;
		aligned_uint64_t	info;
	} info;

	struct { /* anonymous struct used by BPF_PROG_QUERY command */
		uint32_t		target_fd;	/* container object to query */
		uint32_t		attach_type;
		uint32_t		query_flags;
		uint32_t		attach_flags;
		aligned_uint64_t	prog_ids;
		uint32_t		prog_cnt;
	} query;

	struct {
		uint64_t name;
		uint32_t prog_fd;
	} raw_tracepoint;

	struct { /* anonymous struct for BPF_BTF_LOAD */
		aligned_uint64_t	btf;
		aligned_uint64_t	btf_log_buf;
		uint32_t		btf_size;
		uint32_t		btf_log_size;
		uint32_t		btf_log_level;
	};

	struct {
		uint32_t		pid;		/* input: pid */
		uint32_t		fd;		/* input: fd */
		uint32_t		flags;		/* input: flags */
		uint32_t		buf_len;	/* input/output: buf len */
		aligned_uint64_t	buf;		/* input/output:
						 *   tp_name for tracepoint
						 *   symbol for kprobe
						 *   filename for uprobe
						 */
		uint32_t		prog_id;	/* output: prod_id */
		uint32_t		fd_type;	/* output: BPF_FD_TYPE_* */
		uint64_t		probe_offset;	/* output: probe_offset */
		uint64_t		probe_addr;	/* output: probe_addr */
	} task_fd_query;
} __attribute__((aligned(8)));

/* End copy from linux/bpf.h */

/* Start copy from tools/lib/bpf  */
inline uint64_t ptr_to_u64(const void *ptr)
{
	return (uint64_t) (unsigned long) ptr;
}

inline int bpf(enum bpf_cmd cmd, union bpf_attr *attr, unsigned int size)
{
	return tst_syscall(__NR_bpf, cmd, attr, size);
}
/* End copy from tools/lib/bpf */

#endif	/* BPF_H */
