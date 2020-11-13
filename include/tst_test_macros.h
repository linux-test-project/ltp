// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2020 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_TEST_MACROS_H__
#define TST_TEST_MACROS_H__

#define TEST(SCALL) \
	do { \
		errno = 0; \
		TST_RET = SCALL; \
		TST_ERR = errno; \
	} while (0)

#define TEST_VOID(SCALL) \
	do { \
		errno = 0; \
		SCALL; \
		TST_ERR = errno; \
	} while (0)

extern long TST_RET;
extern int TST_ERR;
extern int TST_PASS;

extern void *TST_RET_PTR;

#define TESTPTR(SCALL) \
	do { \
		errno = 0; \
		TST_RET_PTR = (void*)SCALL; \
		TST_ERR = errno; \
	} while (0)


#define TST_2_(_1, _2, ...) _2

#define TST_FMT_(FMT, _1, ...) FMT, ##__VA_ARGS__

#define TST_MSG_(RES, FMT, SCALL, ...) \
	tst_res_(__FILE__, __LINE__, RES, \
		TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, SCALL) FMT, __VA_ARGS__))

#define TST_MSGP_(RES, FMT, PAR, SCALL, ...) \
	tst_res_(__FILE__, __LINE__, RES, \
		TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, SCALL) FMT, __VA_ARGS__), PAR)

#define TST_EXP_FD(SCALL, ...)                                                 \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET == -1) {                                           \
			TST_MSG_(TFAIL | TTERRNO, " failed",                   \
			         #SCALL, ##__VA_ARGS__);                       \
			break;                                                 \
		}                                                              \
		                                                               \
		if (TST_RET < 0) {                                             \
			TST_MSGP_(TFAIL | TTERRNO, " invalid retval %ld",      \
			          TST_RET, #SCALL, ##__VA_ARGS__);             \
			break;                                                 \
		}                                                              \
                                                                               \
		TST_MSGP_(TPASS, " returned fd %ld", TST_RET,                  \
		         #SCALL, ##__VA_ARGS__);                               \
                                                                               \
		TST_PASS = 1;                                                  \
                                                                               \
	} while (0)

#define TST_EXP_PASS(SCALL, ...)                                               \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET == -1) {                                           \
			TST_MSG_(TFAIL | TTERRNO, " failed",                   \
			         #SCALL, ##__VA_ARGS__);                       \
		        break;                                                 \
		}                                                              \
		                                                               \
		if (TST_RET != 0) {                                            \
			TST_MSGP_(TFAIL | TTERRNO, " invalid retval %ld",      \
			          TST_RET, #SCALL, ##__VA_ARGS__);             \
			break;                                                 \
		}                                                              \
                                                                               \
		TST_MSG_(TPASS, " passed", #SCALL, ##__VA_ARGS__);             \
                                                                               \
		TST_PASS = 1;                                                  \
                                                                               \
	} while (0)


#define TST_EXP_FAIL(SCALL, ERRNO, ...)                                        \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET == 0) {                                            \
			TST_MSG_(TFAIL | TTERRNO, " succeeded",                \
			         #SCALL, ##__VA_ARGS__);                       \
		        break;                                                 \
		}                                                              \
		                                                               \
		if (TST_RET != -1) {                                           \
			TST_MSGP_(TFAIL | TTERRNO, " invalid retval %ld",      \
			          TST_RET, #SCALL, ##__VA_ARGS__);             \
			break;                                                 \
		}                                                              \
		                                                               \
		if (ERRNO) {                                                   \
			if (TST_ERR == ERRNO) {                                \
				TST_MSG_(TPASS | TERRNO, "",                   \
				         #SCALL, ##__VA_ARGS__);               \
				TST_PASS = 1;                                  \
			} else {                                               \
				TST_MSGP_(TFAIL | TERRNO, " expected %s",      \
				          tst_strerrno(ERRNO),                 \
				          #SCALL, ##__VA_ARGS__);              \
			}                                                      \
		}                                                              \
	} while (0)

#endif	/* TST_TEST_MACROS_H__ */
