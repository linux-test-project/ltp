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

#define TST_EXP_POSITIVE_(SCALL, SSCALL, ...)                                  \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET == -1) {                                           \
			TST_MSG_(TFAIL | TTERRNO, " failed",                   \
			         SSCALL, ##__VA_ARGS__);                       \
			break;                                                 \
		}                                                              \
		                                                               \
		if (TST_RET < 0) {                                             \
			TST_MSGP_(TFAIL | TTERRNO, " invalid retval %ld",      \
			          TST_RET, SSCALL, ##__VA_ARGS__);             \
			break;                                                 \
		}                                                              \
                                                                               \
		TST_PASS = 1;                                                  \
                                                                               \
	} while (0)

#define TST_EXP_POSITIVE(SCALL, ...)                               \
	do {                                                       \
		TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__);   \
		                                                   \
		if (TST_PASS) {                                    \
			TST_MSGP_(TPASS, " returned %ld",          \
			          TST_RET, #SCALL, ##__VA_ARGS__); \
		}                                                  \
	} while (0)

#define TST_EXP_FD_SILENT(SCALL, ...)	TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__)

#define TST_EXP_FD(SCALL, ...)                                                 \
	do {                                                                   \
		TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__);               \
		                                                               \
		if (TST_PASS)                                                  \
			TST_MSGP_(TPASS, " returned fd %ld", TST_RET,          \
				#SCALL, ##__VA_ARGS__);                        \
	} while (0)

#define TST_EXP_PID_SILENT(SCALL, ...)	TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__)

#define TST_EXP_PID(SCALL, ...)                                                \
	do {                                                                   \
		TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__);               \
									       \
		if (TST_PASS)                                                  \
			TST_MSGP_(TPASS, " returned pid %ld", TST_RET,         \
				#SCALL, ##__VA_ARGS__);                        \
	} while (0)

#define TST_EXP_VAL_SILENT_(SCALL, VAL, SSCALL, ...)                           \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET != VAL) {                                          \
			TST_MSGP_(TFAIL | TTERRNO, " retval not %ld",          \
			          (long )VAL, SSCALL, ##__VA_ARGS__);          \
			break;                                                 \
		}                                                              \
		                                                               \
		TST_PASS = 1;                                                  \
		                                                               \
	} while (0)

#define TST_EXP_VAL_SILENT(SCALL, VAL, ...) TST_EXP_VAL_SILENT_(SCALL, VAL, #SCALL, ##__VA_ARGS__)

#define TST_EXP_VAL(SCALL, VAL, ...)                                           \
	do {                                                                   \
		TST_EXP_VAL_SILENT_(SCALL, VAL, #SCALL, ##__VA_ARGS__);        \
		                                                               \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS, " passed", #SCALL, ##__VA_ARGS__);     \
			                                                       \
	} while(0)

#define TST_EXP_PASS_SILENT_(SCALL, SSCALL, ...)                               \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET == -1) {                                           \
			TST_MSG_(TFAIL | TTERRNO, " failed",                   \
			         SSCALL, ##__VA_ARGS__);                       \
		        break;                                                 \
		}                                                              \
		                                                               \
		if (TST_RET != 0) {                                            \
			TST_MSGP_(TFAIL | TTERRNO, " invalid retval %ld",      \
			          TST_RET, SSCALL, ##__VA_ARGS__);             \
			break;                                                 \
		}                                                              \
                                                                               \
		TST_PASS = 1;                                                  \
                                                                               \
	} while (0)

#define TST_EXP_PASS_SILENT(SCALL, ...) TST_EXP_PASS_SILENT_(SCALL, #SCALL, ##__VA_ARGS__)

#define TST_EXP_PASS(SCALL, ...)                                               \
	do {                                                                   \
		TST_EXP_PASS_SILENT_(SCALL, #SCALL, ##__VA_ARGS__);            \
		                                                               \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS, " passed", #SCALL, ##__VA_ARGS__);     \
	} while (0)                                                            \

#define TST_EXP_FAIL_(PASS_COND, SCALL, SSCALL, ERRNO, ...)                    \
	do {                                                                   \
		TEST(SCALL);                                                   \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (PASS_COND) {                                               \
			TST_MSG_(TFAIL, " succeeded", SSCALL, ##__VA_ARGS__);  \
		        break;                                                 \
		}                                                              \
		                                                               \
		if (TST_RET != -1) {                                           \
			TST_MSGP_(TFAIL | TTERRNO, " invalid retval %ld",      \
			          TST_RET, SSCALL, ##__VA_ARGS__);             \
			break;                                                 \
		}                                                              \
		                                                               \
		if (TST_ERR == ERRNO) {					\
			TST_MSG_(TPASS | TTERRNO, " ",			\
				 SSCALL, ##__VA_ARGS__);		\
			TST_PASS = 1;					\
		} else {						\
			TST_MSGP_(TFAIL | TTERRNO, " expected %s",	\
				  tst_strerrno(ERRNO),			\
				  SSCALL, ##__VA_ARGS__);		\
		}							\
	} while (0)

#define TST_EXP_FAIL(SCALL, ERRNO, ...) TST_EXP_FAIL_(TST_RET == 0, SCALL, #SCALL, ERRNO, ##__VA_ARGS__)

#define TST_EXP_FAIL2(SCALL, ERRNO, ...) TST_EXP_FAIL_(TST_RET >= 0, SCALL, #SCALL, ERRNO, ##__VA_ARGS__)

#endif	/* TST_TEST_MACROS_H__ */
