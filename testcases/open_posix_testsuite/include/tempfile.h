// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Joerg Vehlow <lkml@jv-coder.de>
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define PTS_GET_TMP_FILENAME(target, prefix) \
    snprintf(target, sizeof(target), \
    "%s/" prefix "_pid-%d", pts_get_tmpdir(), getpid());

static inline const char *pts_get_tmpdir(void)
{
    const char *tmpdir_env;
    tmpdir_env = getenv("TMPDIR");
    return tmpdir_env ? tmpdir_env : "/tmp";
}
