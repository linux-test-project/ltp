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

#define PTS_PASS        0
#define PTS_FAIL        1
#define PTS_UNRESOLVED  2
#define PTS_UNSUPPORTED 4
#define PTS_UNTESTED    5

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

/* __attribute__ is a non portable gcc extension */
/* TODO: Add support for C23 attributes */
#if defined __has_attribute
#  if __has_attribute(noreturn)
#    define PTS_ATTRIBUTE_NORETURN      __attribute__((noreturn))
#  endif
#  if __has_attribute(unused)
#    define PTS_ATTRIBUTE_UNUSED        __attribute__((unused))
#  endif
#  if __has_attribute(warn_unused_result)
#    define PTS_ATTRIBUTE_UNUSED_RESULT __attribute__((warn_unused_result))
#  endif
#endif

#ifndef PTS_ATTRIBUTE_NORETURN
#define PTS_ATTRIBUTE_NORETURN
#endif
#ifndef PTS_ATTRIBUTE_UNUSED
#define PTS_ATTRIBUTE_UNUSED
#endif
#ifndef PTS_ATTRIBUTE_UNUSED_RESULT
#define PTS_ATTRIBUTE_UNUSED_RESULT
#endif


#define PTS_WRITE_MSG(msg) do { \
         if (write(STDOUT_FILENO, msg, sizeof(msg) - 1)) { \
                 /* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425 */ \
         } \
} while (0)
