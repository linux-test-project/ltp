/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the sys/mman.h header file exists when the implementation supports
 * at least one of the following options:
 *  MF
 *  SHM
 *  ML
 *  MPR
 *  TYM
 *  SIO
 *  ADV
 */

#include <unistd.h>

#if defined(_POSIX_ADVISORY_INFO) || defined(_POSIX_MAPPED_FILES) || \
    defined(_POSIX_MEMLOCK) || defined(_POSIX_MEMORY_PROTECTION) || \
    defined(_POSIX_SHARED_MEMORY_OBJECTS) || \
    defined(_POSIX_SYNCHRONIZED_IO) || defined(_POSIX_TYPED_MEMORY_OBJECTS)

#include <sys/mman.h>

#endif
