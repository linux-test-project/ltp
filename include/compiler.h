/*
 * compiler.h: take care of fun compiler details here
 *
 * Licensed under the GPL-2 or later
 */

#ifndef __LTP_COMPILER_H__
#define __LTP_COMPILER_H__

#define LTP_ATTRIBUTE_NORETURN __attribute__((noreturn))
#define LTP_ATTRIBUTE_UNUSED __attribute__((unused))

#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#endif /* __LTP_COMPILER_H__ */
