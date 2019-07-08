// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#ifndef TST_COREDUMP__
#define TST_COREDUMP__

/*
 * If crash is expected, avoid dumping corefile.
 * 1 is a special value, that disables core-to-pipe.
 * At the same time it is small enough value for
 * core-to-file, so it skips creating cores as well.
 */
void tst_no_corefile(int verbose);

#endif /* TST_COREDUMP_H */

