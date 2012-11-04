/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the header declares struct posix_typed_mem_info with at least the
 * following member:
 *  size_t posix_tmi_length
 *
 * @pt:TYM
 */

#include <sys/mman.h>
#include <unistd.h>

#if defined(_POSIX_TYPED_MEMORY_OBJECTS) && _POSIX_TYPED_MEMORY_OBJECTS != -1

struct posix_typed_mem_info this_type_should_exist, t;

int dummyfcn(void)
{
	size_t sz = 0;

	t.posix_tmi_length = sz;

	return 0;
}

#endif
