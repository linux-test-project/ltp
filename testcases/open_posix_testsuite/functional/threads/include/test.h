
/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content
 *  of this license, see the COPYING file at the top level of this
 *  source tree.
 */

#include <errno.h>
#include <string.h>

#define EOWNERDEAD	ESRCH
#define ENOTRECOVERABLE	EBADR

#define PASS	0
#define FAIL	1
#define UNRESOLVED 2

#define DPRINTF(a, x, args...)     fprintf(a, x , ##args);
#define EPRINTF(x, args...)	fprintf(stderr, "%s: %d: " x "\n",__FILE__, __LINE__, ##args);

