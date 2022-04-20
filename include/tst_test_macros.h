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

#define _TST_TOSTR(STR) #STR
#define TST_TOSTR(STR) _TST_TOSTR(STR)

#define TST_FMT_(FMT, _1, ...) FMT, ##__VA_ARGS__

#define TST_MSG_(RES, FMT, SCALL, ...) \
	tst_res_(__FILE__, __LINE__, RES, \
		TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, SCALL) FMT, __VA_ARGS__))

#define TST_MSGP_(RES, FMT, PAR, SCALL, ...) \
	tst_res_(__FILE__, __LINE__, RES, \
		TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, SCALL) FMT, __VA_ARGS__), PAR)

#define TST_MSGP2_(RES, FMT, PAR, PAR2, SCALL, ...) \
	tst_res_(__FILE__, __LINE__, RES, \
		TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, SCALL) FMT, __VA_ARGS__), PAR, PAR2)

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

#define TST_EXP_POSITIVE(SCALL, ...)                                           \
	do {                                                                   \
		TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__);               \
		                                                               \
		if (TST_PASS) {                                                \
			TST_MSGP_(TPASS, " returned %ld",                      \
			          TST_RET, #SCALL, ##__VA_ARGS__);             \
		}                                                              \
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
			TST_MSGP2_(TFAIL | TTERRNO, " retval %ld != %ld",      \
			          TST_RET, (long)VAL, SSCALL, ##__VA_ARGS__);  \
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

#define TST_EXP_FAIL_SILENT_(PASS_COND, SCALL, SSCALL, ERRNO, ...)             \
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
		if (TST_ERR == (ERRNO)) {                                      \
			TST_PASS = 1;                                          \
		} else {                                                       \
			TST_MSGP_(TFAIL | TTERRNO, " expected %s",             \
				  tst_strerrno(ERRNO),                         \
				  SSCALL, ##__VA_ARGS__);                      \
		}                                                              \
	} while (0)

#define TST_EXP_FAIL(SCALL, ERRNO, ...)                                        \
	do {                                                                   \
		TST_EXP_FAIL_SILENT_(TST_RET == 0, SCALL, #SCALL,              \
			ERRNO, ##__VA_ARGS__);                                 \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS | TTERRNO, " ", #SCALL, ##__VA_ARGS__); \
	} while (0)

#define TST_EXP_FAIL2(SCALL, ERRNO, ...)                                       \
	do {                                                                   \
		TST_EXP_FAIL_SILENT_(TST_RET >= 0, SCALL, #SCALL,              \
			ERRNO, ##__VA_ARGS__);                                 \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS | TTERRNO, " ", #SCALL, ##__VA_ARGS__); \
	} while (0)

#define TST_EXP_FAIL_SILENT(SCALL, ERRNO, ...) \
	TST_EXP_FAIL_SILENT_(TST_RET == 0, SCALL, #SCALL, ERRNO, ##__VA_ARGS__)

#define TST_EXP_FAIL2_SILENT(SCALL, ERRNO, ...) \
	TST_EXP_FAIL_SILENT_(TST_RET >= 0, SCALL, #SCALL, ERRNO, ##__VA_ARGS__)

#define TST_EXP_EXPR(EXPR, FMT, ...)						\
	tst_res_(__FILE__, __LINE__, (EXPR) ? TPASS : TFAIL, "Expect: " FMT, ##__VA_ARGS__);

#define TST_EXP_EQ_(VAL_A, SVAL_A, VAL_B, SVAL_B, TYPE, PFS) do {\
	TYPE tst_tmp_a__ = VAL_A; \
	TYPE tst_tmp_b__ = VAL_B; \
	if (tst_tmp_a__ == tst_tmp_b__) { \
		tst_res_(__FILE__, __LINE__, TPASS, \
			SVAL_A " == " SVAL_B " (" PFS ")", tst_tmp_a__); \
	} else { \
		tst_res_(__FILE__, __LINE__, TFAIL, \
			SVAL_A " (" PFS ") != " SVAL_B " (" PFS ")", \
			tst_tmp_a__, tst_tmp_b__); \
	} \
} while (0)

#define TST_EXP_EQ_LI(VAL_A, VAL_B) \
		TST_EXP_EQ_(VAL_A, #VAL_A, VAL_B, #VAL_B, long long, "%lli")

#define TST_EXP_EQ_LU(VAL_A, VAL_B) \
		TST_EXP_EQ_(VAL_A, #VAL_A, VAL_B, #VAL_B, unsigned long long, "%llu")

#define TST_EXP_EQ_SZ(VAL_A, VAL_B) \
		TST_EXP_EQ_(VAL_A, #VAL_A, VAL_B, #VAL_B, size_t, "%zu")

#define TST_EXP_EQ_SSZ(VAL_A, VAL_B) \
		TST_EXP_EQ_(VAL_A, #VAL_A, VAL_B, #VAL_B, ssize_t, "%zi")

#endif	/* TST_TEST_MACROS_H__ */
