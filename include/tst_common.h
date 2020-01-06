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
 * @ERET - The value returned from @FUNC on success
 *
 * This macro will call @FUNC in a loop with a delay between retries. If @FUNC
 * returns @ERET then the loop exits. The delay between retries starts at one
 * micro second and is then doubled each iteration until it exceeds one second
 * (the total time sleeping will be approximately one second as well). When the
 * delay exceeds one second tst_brk() is called.
 */
#define TST_RETRY_FUNC(FUNC, ERET) \
	TST_RETRY_FN_EXP_BACKOFF(FUNC, ERET, 1)

#define TST_RETRY_FN_EXP_BACKOFF(FUNC, ERET, MAX_DELAY)	\
({	unsigned int tst_delay_, tst_max_delay_;			\
	tst_delay_ = 1;							\
	tst_max_delay_ = tst_multiply_timeout(MAX_DELAY * 1000000);	\
	for (;;) {							\
		typeof(FUNC) tst_ret_ = FUNC;				\
		if (tst_ret_ == ERET)					\
			break;						\
		if (tst_delay_ < tst_max_delay_) {			\
			usleep(tst_delay_);				\
			tst_delay_ *= 2;				\
		} else {						\
			tst_brk(TBROK, #FUNC" timed out");		\
		}							\
	}								\
	ERET;								\
})

#define TST_BRK_SUPPORTS_ONLY_TCONF_TBROK(condition) \
	do { ((void)sizeof(char[1 - 2 * !!(condition)])); } while (0)

#endif /* TST_COMMON_H__ */
