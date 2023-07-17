/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2020 Linux Test Project
 */

#ifndef LTP_BPF_COMMON_H
#define LTP_BPF_COMMON_H

#include <sys/types.h>
#include <inttypes.h>

#include "lapi/bpf.h"
#include "lapi/socket.h"

#define BPF_MEMLOCK_ADD (2*1024*1024)
#define BUFSIZE 8192

/* map[array_indx] = reg_to_save
 *
 * Inserts the following instructions
 *
 * r1 = map_fd
 * r2 = fp
 * r2 = r2 - 4
 * r2 = array_indx
 * call map_lookup_elem(r1, r2)
 * if r0 != 0 goto pc+1
 * exit
 * *r0 = reg_to_save
 *
 */
#define BPF_MAP_ARRAY_STX(map_fd, array_indx, reg_to_save)\
	BPF_LD_MAP_FD(BPF_REG_1, map_fd),		\
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),		\
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),		\
	BPF_ST_MEM(BPF_W, BPF_REG_2, 0, array_indx),	\
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),	\
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),		\
	BPF_EXIT_INSN(),				\
	BPF_STX_MEM(BPF_DW, BPF_REG_0, reg_to_save, 0)

void rlimit_bump_memlock(void);

int bpf_map_create(union bpf_attr *const attr)
	__attribute__((nonnull, warn_unused_result));
int bpf_map_array_create(const uint32_t max_entries)
	__attribute__((warn_unused_result));
void bpf_map_array_get(const int map_fd,
		       const uint32_t *const array_indx,
		       uint64_t *const array_val)
	__attribute__((nonnull));

void bpf_init_prog_attr(union bpf_attr *const attr,
			const struct bpf_insn *const prog,
			const size_t prog_size,
			char *const log_buf, const size_t log_size)
	__attribute__((nonnull));
int bpf_load_prog(union bpf_attr *const attr, const char *const log)
	__attribute__((nonnull, warn_unused_result));
void bpf_run_prog(const int prog_fd,
		  const char *const msg, const size_t msg_len)
	__attribute__((nonnull));

#endif
