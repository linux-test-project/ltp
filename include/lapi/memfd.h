/*
 * Copyright (C) 2017  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
