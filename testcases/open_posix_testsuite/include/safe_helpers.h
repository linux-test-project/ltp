/*
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * alone with this program.
 */

#ifndef __SAFE_HELPERS_H__
#define __SAFE_HELPERS_H__

#include <stdio.h>
#include <string.h>

#define SAFE_PFUNC(op) \
do {\
	int ret = (op); \
	if (ret != 0) { \
		printf("Test %s unresolved: got %i (%s) on line %i\n  %s\n", \
			__FILE__, ret, strerror(ret), __LINE__, #op); \
		fflush(stdout); \
		exit(PTS_UNRESOLVED); \
	} \
} while (0)

#define SAFE_FUNC(op) \
({ \
	int ret = (op); \
	if (ret == -1) { \
		printf("Test %s unresolved: got %i (%s) on line %i\n  %s\n", \
			__FILE__, ret, strerror(errno), __LINE__, #op); \
		fflush(stdout); \
		exit(PTS_UNRESOLVED); \
	} \
	ret; \
})

#endif
