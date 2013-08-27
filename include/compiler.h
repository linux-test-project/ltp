/*
 * compiler.h: take care of fun compiler details here
 *
 * Licensed under the GPL-2 or later
 */

#ifndef __LTP_COMPILER_H__
#define __LTP_COMPILER_H__

#define LTP_ATTRIBUTE_NORETURN		__attribute__((noreturn))
#define LTP_ATTRIBUTE_UNUSED		__attribute__((unused))
#define LTP_ATTRIBUTE_UNUSED_RESULT	__attribute__((warn_unused_result))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

/* Round x to the next multiple of a.
 * a should be a power of 2.
 */
#define LTP_ALIGN(x, a)    __LTP_ALIGN_MASK(x, (typeof(x))(a) - 1)
#define __LTP_ALIGN_MASK(x, mask)  (((x) + (mask)) & ~(mask))

#endif /* __LTP_COMPILER_H__ */
