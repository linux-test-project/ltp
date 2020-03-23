/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2013-2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_SAFE_STDIO_H__
#define TST_SAFE_STDIO_H__

#include <stdio.h>

#include "safe_stdio_fn.h"

#define SAFE_FOPEN(path, mode) \
	safe_fopen(__FILE__, __LINE__, NULL, path, mode)

#define SAFE_FCLOSE(f) \
	safe_fclose(__FILE__, __LINE__, NULL, f)

#define SAFE_ASPRINTF(strp, fmt, ...) \
	safe_asprintf(__FILE__, __LINE__, NULL, strp, fmt, __VA_ARGS__)

#define SAFE_POPEN(command, type) \
	safe_popen(__FILE__, __LINE__, NULL, command, type)

#endif /* TST_SAFE_STDIO_H__ */
