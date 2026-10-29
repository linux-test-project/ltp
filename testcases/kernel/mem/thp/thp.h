// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2011-2021
 * Copyright (c) Cyril Hrubis 2024
 */
#ifndef THP_H
#define THP_H

#include "tst_path_defs.h"

static inline void check_hugepage(void)
{
        if (access(PATH_MM_HUGEPAGES, F_OK))
                tst_brk(TCONF, "Huge page is not supported.");
}

#endif /* THP_H */
