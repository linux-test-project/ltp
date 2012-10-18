/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the header defines the POSIX_TYPED_MEM_ALLOCATE_CONTIG flag for
 * posix_typed_mem_open()().
 *
 * @pt:TYM
 */

#include <sys/mman.h>
#include <unistd.h>

#if defined(_POSIX_TYPED_MEMORY_OBJECTS) && _POSIX_TYPED_MEMORY_OBJECTS != -1

#ifndef POSIX_TYPED_MEM_ALLOCATE_CONTIG
#error POSIX_TYPED_MEM_ALLOCATE_CONTIG not defined
#endif

#endif
