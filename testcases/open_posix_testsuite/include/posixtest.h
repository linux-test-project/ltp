/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * return codes
 */

/*
 * Define PTS_DEVELOPER_MODE if you want to compile for developer scenarios,
 * including reporting errors (as opposed to warnings), when compiling some
 * test programs.
 */

#if defined(_GNU_SOURCE)
# if !AFFINITY_NEEDS_GNU_SOURCE
#  if defined(PTS_DEVELOPER_MODE)
#   error "Contains GNU-isms that need fixing."
#  else
#   warning "Contains GNU-isms that need fixing."
#  endif
# endif
#endif

#if defined(_BSD_SOURCE)
# if defined(PTS_DEVELOPER_MODE)
#  error "Contains BSD-isms that need fixing."
# else
#  warning "Contains BSD-isms that need fixing."
# endif
#endif

#define PTS_PASS        0
#define PTS_FAIL        1
#define PTS_UNRESOLVED  2
#define PTS_UNSUPPORTED 4
#define PTS_UNTESTED    5

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif
