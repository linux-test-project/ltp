// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Andrea Cervesato <andrea.cervesato@suse.com>
 * Copyright (C) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_FICLONE_H__
#define LAPI_FICLONE_H__

#include "config.h"
#include <linux/fs.h>
#include <stdint.h>

#ifndef HAVE_STRUCT_FILE_CLONE_RANGE
struct file_clone_range {
	int64_t src_fd;
	uint64_t src_offset;
	uint64_t src_length;
	uint64_t dest_offset;
};
#endif

#ifndef FICLONE
# define FICLONE		_IOW(0x94, 9, int)
#endif

#ifndef FICLONERANGE
# define FICLONERANGE		_IOW(0x94, 13, struct file_clone_range)
#endif

#endif /* LAPI_FICLONE_H__ */
