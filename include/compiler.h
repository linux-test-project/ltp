/*
 * compiler.h: take care of fun compiler details here
 *
 * Licensed under the GPL-2 or later
 */

#ifndef __LTP_COMPILER_H__
#define __LTP_COMPILER_H__

#define attribute_noreturn __attribute__((noreturn))

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#endif
