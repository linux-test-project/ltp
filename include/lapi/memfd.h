// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017  Red Hat, Inc.
 */

#ifndef LAPI_MEMFD_H
#define LAPI_MEMFD_H

/* flags for memfd_create(2) (unsigned int) */
#ifndef MFD_CLOEXEC
# define MFD_CLOEXEC             0x0001U
#endif
#ifndef MFD_ALLOW_SEALING
# define MFD_ALLOW_SEALING       0x0002U
#endif

/* flags for memfd_create(3) and memfd_create(4) */
#ifndef MFD_HUGETLB
#define MFD_HUGETLB 0x0004U
#endif

#ifndef MFD_HUGE_64KB
#define MFD_HUGE_64KB (16 << 26)
#endif
#ifndef MFD_HUGE_512KB
#define MFD_HUGE_512KB (19 << 26)
#endif
#ifndef MFD_HUGE_2MB
#define MFD_HUGE_2MB (21 << 26)
#endif
#ifndef MFD_HUGE_8MB
#define MFD_HUGE_8MB (23 << 26)
#endif
#ifndef MFD_HUGE_16MB
#define MFD_HUGE_16MB (24 << 26)
#endif
#ifndef MFD_HUGE_256MB
#define MFD_HUGE_256MB (28 << 26)
#endif
#ifndef MFD_HUGE_1GB
#define MFD_HUGE_1GB (30 << 26)
#endif
#ifndef MFD_HUGE_2GB
#define MFD_HUGE_2GB (31 << 26)
#endif
#ifndef MFD_HUGE_16GB
#define MFD_HUGE_16GB (34 << 26)
#endif

#endif
