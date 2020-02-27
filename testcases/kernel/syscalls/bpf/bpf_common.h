/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2020 Linux Test Project
 */

#ifndef LTP_BPF_COMMON_H
#define LTP_BPF_COMMON_H

#define BPF_MEMLOCK_ADD (2*1024*1024)

void rlimit_bump_memlock(void);
int bpf_map_create(union bpf_attr *attr);
void bpf_init_prog_attr(union bpf_attr *attr, const struct bpf_insn *prog,
	size_t prog_size, char *log_buf, size_t log_size);
int bpf_load_prog(union bpf_attr *attr, const char *log);

#endif
