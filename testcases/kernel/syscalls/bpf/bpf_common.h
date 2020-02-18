/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2020 Linux Test Project
 */

#ifndef LTP_BPF_COMMON_H
#define LTP_BPF_COMMON_H

#define BPF_MEMLOCK_ADD (256*1024)

void rlimit_bump_memlock(void);
int bpf_map_create(union bpf_attr *attr);

#endif
