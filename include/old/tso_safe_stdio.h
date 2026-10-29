// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013-2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef TSO_SAFE_STDIO_H__
#define TSO_SAFE_STDIO_H__

#include <stdio.h>

#include "safe_stdio_fn.h"

#define SAFE_FOPEN(cleanup_fn, path, mode) \
	safe_fopen(__FILE__, __LINE__, cleanup_fn, path, mode)

#define SAFE_FCLOSE(cleanup_fn, f) \
	safe_fclose(__FILE__, __LINE__, cleanup_fn, f)

#define SAFE_ASPRINTF(cleanup_fn, strp, fmt, ...) \
	safe_asprintf(__FILE__, __LINE__, cleanup_fn, strp, fmt, __VA_ARGS__)

#define SAFE_POPEN(cleanup_fn, command, type) \
	safe_popen(__FILE__, __LINE__, cleanup_fn, command, type)

#endif /* TSO_SAFE_STDIO_H__ */
