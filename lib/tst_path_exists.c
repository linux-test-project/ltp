// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2011-2021
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2024
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include "tst_fs.h"

int tst_path_exists(const char *fmt, ...)
{
        va_list ap;
        char pathbuf[PATH_MAX];

        va_start(ap, fmt);
        vsnprintf(pathbuf, sizeof(pathbuf), fmt, ap);
        va_end(ap);

        return access(pathbuf, F_OK) == 0;
}
