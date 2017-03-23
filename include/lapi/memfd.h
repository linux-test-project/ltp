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

#endif
