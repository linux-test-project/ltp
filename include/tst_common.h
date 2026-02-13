// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2013 Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 * Copyright (c) 2010 Ngie Cooper <yaneurabeya@gmail.com>
 * Copyright (c) 2008 Mike Frysinger <vapier@gmail.com>
 */

#ifndef TST_COMMON_H__
#define TST_COMMON_H__

#define LTP_ATTRIBUTE_NORETURN		__attribute__((noreturn))
#define LTP_ATTRIBUTE_UNUSED		__attribute__((unused))
#define LTP_ATTRIBUTE_UNUSED_RESULT	__attribute__((warn_unused_result))

#define LTP_VAR_USED(p) asm volatile("" :: "m"(p)); p

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/* Round x to the next multiple of a.
 * a should be a power of 2.
 */
#define LTP_ALIGN(x, a)    __LTP_ALIGN_MASK(x, (typeof(x))(a) - 1)
#define __LTP_ALIGN_MASK(x, mask)  (((x) + (mask)) & ~(mask))

/**
 * TST_RETRY_FUNC() - Repeatedly retry a function with an increasing delay.
 * @FUNC - The function which will be retried
 * @ECHCK - Function/macro for validating @FUNC return value
 *
 * This macro will call @FUNC in a loop with a delay between retries.
 * If ECHCK(ret) evaluates to non-zero, the loop ends. The delay between
 * retries starts at one microsecond and is then doubled each iteration until
 * it exceeds one second (the total time sleeping will be approximately one
 * second as well). When the delay exceeds one second, the loop will end.
 * The TST_RETRY_FUNC() macro returns the last value returned by @FUNC.
 */
#define TST_RETRY_FUNC(FUNC, ECHCK) \
	TST_RETRY_FN_EXP_BACKOFF(FUNC, ECHCK, 1)

#define TST_RETRY_FN_EXP_BACKOFF(FUNC, ECHCK, MAX_DELAY)	\
({	unsigned int tst_delay_, tst_max_delay_;			\
	typeof(FUNC) tst_ret_;						\
	tst_delay_ = 1;							\
	tst_max_delay_ = tst_multiply_timeout(MAX_DELAY * 1000000);	\
	for (;;) {							\
		errno = 0;						\
		tst_ret_ = FUNC;					\
		if (ECHCK(tst_ret_))					\
			break;						\
		if (tst_delay_ < tst_max_delay_) {			\
			usleep(tst_delay_);				\
			tst_delay_ *= 2;				\
		} else {						\
			break;						\
		}							\
	}								\
	tst_ret_;							\
})

/*
 * Return value validation macros for TST_RETRY_FUNC():
 * TST_RETVAL_EQ0() - Check that value is equal to zero
 */
#define TST_RETVAL_EQ0(x) (!(x))

/*
 * TST_RETVAL_NOTNULL() - Check that value is not equal to zero/NULL
 */
#define TST_RETVAL_NOTNULL(x) (!!(x))

/*
 * TST_RETVAL_GE0() - Check that value is greater than or equal to zero
 */
#define TST_RETVAL_GE0(x) ((x) >= 0)

#define TST_BUILD_BUG_ON(condition) \
	do { ((void)sizeof(char[1 - 2 * !!(condition)])); } while (0)

#define TST_RES_SUPPORTS_TCONF_TDEBUG_TFAIL_TINFO_TPASS_TWARN(condition) \
	TST_BUILD_BUG_ON(condition)

/* stringification */
#define TST_TO_STR_(s) #s
#define TST_TO_STR(s) TST_TO_STR_(s)

/*
 * TST_PTR_TO_UINT - Casts a pointer to a 64-bit unsigned integer.
 */
#define TST_PTR_TO_UINT(x) ((uintptr_t)(x))

#endif /* TST_COMMON_H__ */
