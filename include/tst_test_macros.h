// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2024 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2021-2024
 */

/**
 * DOC: tst_test_macros.h -- helpers for testing syscalls
 */

#ifndef TST_TEST_MACROS_H__
#define TST_TEST_MACROS_H__

#include <stdbool.h>

extern long TST_RET;
extern void *TST_RET_PTR;
extern int TST_ERR;
extern int TST_PASS;

/**
 * TEST() - Store syscall retval long and errno.
 *
 * @SCALL: Tested syscall e.g. write(fd, buf, len).
 *
 * This macro is a shortcut for storing an errno and a return value. The errno is
 * cleared before the syscall is called and saved to a TST_ERR global variable
 * right after the call returns. The return value is available in TST_RET
 * global variable.
 *
 * The TST_ERR and TST_RET can be included into tst_res() messages with the
 * TST_ERR and TTERRNO and TRERRNO flags.
 *
 * This macro is also used as a base for all the more specific variants e.g.
 * TST_EXP_FAIL().
 */
#define TEST(SCALL) \
	do { \
		errno = 0; \
		TST_RET = SCALL; \
		TST_ERR = errno; \
	} while (0)

/**
 * TESTPTR() - Store syscall retval pointer and errno.
 *
 * @SCALL: Tested syscall e.g. write(fd, buf, len).
 *
 * Sets TST_RET_PTR and TST_ERR.
 *
 * This is the same as TEST() macro the only difference is that the return
 * value is a pointer which is stored into TST_RET_PTR instead.
 */
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

#define TST_MSGP2_(RES, FMT, PAR, PAR2, SCALL, ...) \
	tst_res_(__FILE__, __LINE__, RES, \
		TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, SCALL) FMT, __VA_ARGS__), PAR, PAR2)

#define TST_EXP_POSITIVE__(SCALL, SSCALL, ...)                                 \
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

#define TST_EXP_POSITIVE_(SCALL, SSCALL, ...)                                  \
	({                                                                     \
		TST_EXP_POSITIVE__(SCALL, SSCALL, ##__VA_ARGS__);              \
		TST_RET;                                                       \
	})

/**
 * TST_EXP_POSITIVE() - Test syscall, expect return value >= 0.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TEST() macro and additionaly prints pass
 * or fail message. Apart from TST_ERR and TST_RET set by the TEST() macro
 * TST_PASS global variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_POSITIVE(SCALL, ...)                                           \
	({                                                                     \
		TST_EXP_POSITIVE__(SCALL, #SCALL, ##__VA_ARGS__);              \
		                                                               \
		if (TST_PASS) {                                                \
			TST_MSGP_(TPASS, " returned %ld",                      \
			          TST_RET, #SCALL, ##__VA_ARGS__);             \
		}                                                              \
		                                                               \
		TST_RET;                                                       \
	})

/**
 * TST_EXP_FD_SILENT() - Test syscall to return a file descriptor, silent variant.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * Unlike TST_EXP_FD() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_FD_SILENT(SCALL, ...)	TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__)

/**
 * TST_EXP_FD() - Test syscall to return a file descriptor.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * This is a variant of the TST_EXP_POSSITIVE() for a more specific case that
 * the returned value is a file descriptor.
 */
#define TST_EXP_FD(SCALL, ...)                                                 \
	({                                                                     \
		TST_EXP_POSITIVE__(SCALL, #SCALL, ##__VA_ARGS__);              \
		                                                               \
		if (TST_PASS)                                                  \
			TST_MSGP_(TPASS, " returned fd %ld", TST_RET,          \
				#SCALL, ##__VA_ARGS__);                        \
		                                                               \
		TST_RET;                                                       \
	})

/**
 * TST_EXP_FD_OR_FAIL() - Test syscall to return file descriptor or fail with
 * expected errno.
 *
 * @SCALL: Tested syscall.
 * @ERRNO: Expected errno or 0.
 * @...: A printf-like parameters.
 *
 * Expect a file descriptor if errno is 0 otherwise expect a failure with
 * expected errno.
 *
 * Internally it uses TST_EXP_FAIL() and TST_EXP_FD().
 */
#define TST_EXP_FD_OR_FAIL(SCALL, ERRNO, ...)                                  \
	({                                                                     \
		if (ERRNO)                                                     \
			TST_EXP_FAIL(SCALL, ERRNO, ##__VA_ARGS__);             \
		else                                                           \
			TST_EXP_FD(SCALL, ##__VA_ARGS__);                      \
		                                                               \
		TST_RET;                                                       \
	})

/**
 * TST_EXP_PID_SILENT() - Test syscall to return PID, silent variant.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * Unlike TST_EXP_PID() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_PID_SILENT(SCALL, ...)	TST_EXP_POSITIVE_(SCALL, #SCALL, ##__VA_ARGS__)

/**
 * TST_EXP_PID() - Test syscall to return PID.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * This is a variant of the TST_EXP_POSSITIVE() for a more specific case that
 * the returned value is a pid.
 */
#define TST_EXP_PID(SCALL, ...)                                                \
	({                                                                     \
		TST_EXP_POSITIVE__(SCALL, #SCALL, ##__VA_ARGS__);              \
									       \
		if (TST_PASS)                                                  \
			TST_MSGP_(TPASS, " returned pid %ld", TST_RET,         \
				#SCALL, ##__VA_ARGS__);                        \
		                                                               \
		TST_RET;                                                       \
	})

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

/**
 * TST_EXP_VAL_SILENT() - Test syscall to return specified value, silent variant.
 *
 * @SCALL: Tested syscall.
 * @VAL: Expected return value.
 * @...: A printf-like parameters.
 *
 * Unlike TST_EXP_VAL() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_VAL_SILENT(SCALL, VAL, ...) TST_EXP_VAL_SILENT_(SCALL, VAL, #SCALL, ##__VA_ARGS__)

/**
 * TST_EXP_VAL() - Test syscall to return specified value.
 *
 * @SCALL: Tested syscall.
 * @VAL: Expected return value.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TEST() macro and additionaly prints pass
 * or fail message after comparing the returned value againts the expected
 * value. Apart from TST_ERR and TST_RET set by the TEST() macro TST_PASS
 * global variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
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

#define TST_EXP_PASS_SILENT_PTR_(SCALL, SSCALL, FAIL_PTR_VAL, ...)             \
	do {                                                                   \
		TESTPTR(SCALL);                                                \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET_PTR == FAIL_PTR_VAL) {                             \
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

/**
 * TST_EXP_PASS_SILENT() - Test syscall to return 0, silent variant.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * Unlike TST_EXP_PASS() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_PASS_SILENT(SCALL, ...) TST_EXP_PASS_SILENT_(SCALL, #SCALL, ##__VA_ARGS__)

/**
 * TST_EXP_PASS() - Test syscall to return 0.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TEST() macro and additionaly prints pass
 * or fail message after checking the return value againts zero. Apart from
 * TST_ERR and TST_RET set by the TEST() macro TST_PASS global variable is set
 * as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_PASS(SCALL, ...)                                               \
	do {                                                                   \
		TST_EXP_PASS_SILENT_(SCALL, #SCALL, ##__VA_ARGS__);            \
		                                                               \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS, " passed", #SCALL, ##__VA_ARGS__);     \
	} while (0)                                                            \

#define TST_EXP_PASS_PTR_(SCALL, SSCALL, FAIL_PTR_VAL, ...)                    \
	do {                                                                   \
		TST_EXP_PASS_SILENT_PTR_(SCALL, SSCALL,                        \
					FAIL_PTR_VAL, ##__VA_ARGS__);          \
		                                                               \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS, " passed", #SCALL, ##__VA_ARGS__);     \
	} while (0)

/**
 * TST_EXP_PASS_PTR_VOID() - Test syscall to return a valid pointer.
 *
 * @SCALL: Tested syscall.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TESTPTR() macro and additionaly prints
 * pass or fail message after checking the return value against (void *)-1.
 * Apart from TST_ERR and TST_RET_PTR set by the TESTPTR() macro TST_PASS
 * global variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_PASS_PTR_VOID(SCALL, ...)                                      \
               TST_EXP_PASS_PTR_(SCALL, #SCALL, (void *)-1, ##__VA_ARGS__);

/*
 * Returns true if err is in the exp_err array.
 */
bool tst_errno_in_set(int err, const int *exp_errs, int exp_errs_cnt);

/*
 * Fills in the buf with the errno names in the exp_err set. The buf must be at
 * least 20 * exp_errs_cnt bytes long.
 */
const char *tst_errno_names(char *buf, const int *exp_errs, int exp_errs_cnt);

#define TST_EXP_FAIL_SILENT_(PASS_COND, SCALL, SSCALL, ERRNOS, ERRNOS_CNT, ...)\
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
		if (tst_errno_in_set(TST_ERR, ERRNOS, ERRNOS_CNT)) {           \
			TST_PASS = 1;                                          \
		} else {                                                       \
			char tst_str_buf__[ERRNOS_CNT * 20];                   \
			TST_MSGP_(TFAIL | TTERRNO, " expected %s",             \
				  tst_errno_names(tst_str_buf__,               \
						  ERRNOS, ERRNOS_CNT),         \
				  SSCALL, ##__VA_ARGS__);                      \
		}                                                              \
	} while (0)

#define TST_EXP_FAIL_SILENT_PTR_(SCALL, SSCALL, FAIL_PTR_VAL,                  \
	ERRNOS, ERRNOS_CNT, ...)                                               \
	do {                                                                   \
		TESTPTR(SCALL);                                                \
		                                                               \
		TST_PASS = 0;                                                  \
		                                                               \
		if (TST_RET_PTR != FAIL_PTR_VAL) {                             \
			TST_MSG_(TFAIL, " succeeded", SSCALL, ##__VA_ARGS__);  \
		        break;                                                 \
		}                                                              \
		                                                               \
		if (!tst_errno_in_set(TST_ERR, ERRNOS, ERRNOS_CNT)) {          \
			char tst_str_buf__[ERRNOS_CNT * 20];                   \
			TST_MSGP_(TFAIL | TTERRNO, " expected %s",             \
				  tst_errno_names(tst_str_buf__,               \
						  ERRNOS, ERRNOS_CNT),         \
				  SSCALL, ##__VA_ARGS__);                      \
			break;                                                 \
		}                                                              \
                                                                               \
		TST_PASS = 1;                                                  \
                                                                               \
	} while (0)

#define TST_EXP_FAIL_PTR_(SCALL, SSCALL, FAIL_PTR_VAL,                         \
	ERRNOS, ERRNOS_CNT, ...)                                               \
	do {                                                                   \
		TST_EXP_FAIL_SILENT_PTR_(SCALL, SSCALL, FAIL_PTR_VAL,          \
	        ERRNOS, ERRNOS_CNT, ##__VA_ARGS__);                            \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS | TTERRNO, " ", SSCALL, ##__VA_ARGS__); \
	} while (0)


#define TST_EXP_FAIL_ARR_(SCALL, SSCALL, EXP_ERRS, EXP_ERRS_CNT, ...)          \
	do {                                                                   \
		TST_EXP_FAIL_SILENT_(TST_RET == 0, SCALL, SSCALL,              \
			EXP_ERRS, EXP_ERRS_CNT, ##__VA_ARGS__);                \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS | TTERRNO, " ", SSCALL, ##__VA_ARGS__); \
	} while (0)

/**
 * TST_EXP_FAIL() - Test syscall to fail with expected errno.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERR: Expected errno.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TEST() macro and additionaly prints pass
 * or fail message. The check passes if syscall has returned -1 and failed with
 * the specified errno.
 *
 * The SCALL is supposed to return zero on success. For syscalls that return
 * positive number on success TST_EXP_FAIL2() has to be used instead.
 *
 * Apart from TST_ERR and TST_RET set by the TEST() macro TST_PASS global
 * variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_FAIL(SCALL, EXP_ERR, ...)                                      \
	do {                                                                   \
		int tst_exp_err__ = EXP_ERR;                                   \
		TST_EXP_FAIL_ARR_(SCALL, #SCALL, &tst_exp_err__, 1,            \
                                  ##__VA_ARGS__);                              \
	} while (0)

/**
 * TST_EXP_FAIL_ARR() - Test syscall to fail with expected errnos.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERRS: Array of expected errnos.
 * @EXP_ERRS_CNT: Lenght of EXP_ERRS.
 * @...: A printf-like parameters.
 *
 * This is a variant of TST_EXP_FAIL() with an array of possible errors.
 */
#define TST_EXP_FAIL_ARR(SCALL, EXP_ERRS, EXP_ERRS_CNT, ...)                   \
		TST_EXP_FAIL_ARR_(SCALL, #SCALL, EXP_ERRS,                     \
				  EXP_ERRS_CNT, ##__VA_ARGS__);

#define TST_EXP_FAIL2_ARR_(SCALL, SSCALL, EXP_ERRS, EXP_ERRS_CNT, ...)         \
	do {                                                                   \
		TST_EXP_FAIL_SILENT_(TST_RET >= 0, SCALL, SSCALL,              \
			EXP_ERRS, EXP_ERRS_CNT, ##__VA_ARGS__);                \
		if (TST_PASS)                                                  \
			TST_MSG_(TPASS | TTERRNO, " ", SSCALL, ##__VA_ARGS__); \
	} while (0)

/**
 * TST_EXP_FAIL2_ARR() - Test syscall to fail with expected errnos.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERRS: Array of expected errnos.
 * @EXP_ERRS_CNT: Lenght of EXP_ERRS.
 * @...: A printf-like parameters.
 *
 * This is a variant of TST_EXP_FAIL2() with an array of possible errors.
 */
#define TST_EXP_FAIL2_ARR(SCALL, EXP_ERRS, EXP_ERRS_CNT, ...)                \
		TST_EXP_FAIL2_ARR_(SCALL, #SCALL, EXP_ERRS,                    \
		                  EXP_ERRS_CNT, ##__VA_ARGS__);

/**
 * TST_EXP_FAIL_PTR_NULL() - Test syscall to fail with expected errno and return a NULL pointer.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERR: Expected errno.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TESTPTR() macro and additionaly prints
 * pass or fail message after checking the return value against NULL and errno.
 *
 * Apart from TST_ERR and TST_RET_PTR set by the TESTPTR() macro TST_PASS
 * global variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_FAIL_PTR_NULL(SCALL, EXP_ERR, ...)                          \
	do {                                                                   \
		int tst_exp_err__ = EXP_ERR;                                   \
		TST_EXP_FAIL_PTR_(SCALL, #SCALL, NULL,                         \
			&tst_exp_err__, 1, ##__VA_ARGS__);                     \
	} while (0)

/**
 * TST_EXP_FAIL_PTR_NULL_ARR() - Test syscall to fail with expected errnos and return a NULL pointer.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERRS: Array of expected errnos.
 * @EXP_ERRS_CNT: Lenght of EXP_ERRS.
 * @...: A printf-like parameters.
 *
 * This is a variant of TST_EXP_FAIL_PTR_NULL() with an array of possible
 * errors.
 */
#define TST_EXP_FAIL_PTR_NULL_ARR(SCALL, EXP_ERRS, EXP_ERRS_CNT, ...)      \
		TST_EXP_FAIL_PTR_(SCALL, #SCALL, NULL,                         \
			EXP_ERRS, EXP_ERRS_CNT, ##__VA_ARGS__);



/**
 * TST_EXP_FAIL_PTR_VOID() - Test syscall to fail with expected errno and return a (void *)-1 pointer.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERR: Expected errno.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TESTPTR() macro and additionaly prints
 * pass or fail message after checking the return value against (void *)-1 and
 * errno.
 *
 * Apart from TST_ERR and TST_RET_PTR set by the TESTPTR() macro TST_PASS
 * global variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_FAIL_PTR_VOID(SCALL, EXP_ERR, ...)                         \
	do {                                                                   \
		int tst_exp_err__ = EXP_ERR;                                   \
		TST_EXP_FAIL_PTR_(SCALL, #SCALL, (void *)-1,                   \
			&tst_exp_err__, 1, ##__VA_ARGS__);                     \
	} while (0)

/**
 * TST_EXP_FAIL_PTR_VOID_ARR() - Test syscall to fail with expected errnos and return a (void *)-1 pointer.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERRS: Array of expected errnos.
 * @EXP_ERRS_CNT: Lenght of EXP_ERRS.
 * @...: A printf-like parameters.
 *
 * This is a variant of TST_EXP_FAIL_PTR_VOID() with an array of possible
 * errors.
 */
#define TST_EXP_FAIL_PTR_VOID_ARR(SCALL, EXP_ERRS, EXP_ERRS_CNT, ...)      \
		TST_EXP_FAIL_PTR_(SCALL, #SCALL, (void *)-1,                   \
			EXP_ERRS, EXP_ERRS_CNT, ##__VA_ARGS__);

/**
 * TST_EXP_FAIL2() - Test syscall to fail with expected errno.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERR: Expected errno.
 * @...: A printf-like parameters.
 *
 * This macro calls the SCALL with a TEST() macro and additionaly prints pass
 * or fail message. The check passes if syscall has returned -1 and failed with
 * the specified errno.
 *
 * The SCALL is supposed to return possitive number on success e.g. pid or file
 * descriptor. For syscalls that return zero on success TST_EXP_FAIL() has to
 * be used instead.
 *
 * Apart from TST_ERR and TST_RET set by the TEST() macro TST_PASS global
 * variable is set as well based on the outcome.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the first parameter
 * is converted to a string and used instead.
 */
#define TST_EXP_FAIL2(SCALL, EXP_ERR, ...)                                     \
	do {                                                                   \
		int tst_exp_err__ = EXP_ERR;                                   \
		TST_EXP_FAIL2_ARR_(SCALL, #SCALL, &tst_exp_err__, 1,           \
                                  ##__VA_ARGS__);                              \
	} while (0)

/**
 * TST_EXP_FAIL_SILENT() - Test syscall to fail with expected errno, silent variant.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERR: Expected errno.
 * @...: A printf-like parameters.
 *
 * Unlike TST_EXP_FAIL() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_FAIL_SILENT(SCALL, EXP_ERR, ...)                               \
	do {                                                                   \
		int tst_exp_err__ = EXP_ERR;                                   \
		TST_EXP_FAIL_SILENT_(TST_RET == 0, SCALL, #SCALL,              \
			&tst_exp_err__, 1, ##__VA_ARGS__);                     \
	} while (0)

/**
 * TST_EXP_FAIL2_SILENT() - Test syscall to fail with expected errno, silent variant.
 *
 * @SCALL: Tested syscall.
 * @EXP_ERR: Expected errno.
 * @...: A printf-like parameters.
 *
 * Unlike TST_EXP_FAIL2() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_FAIL2_SILENT(SCALL, EXP_ERR, ...)                              \
	do {                                                                   \
		int tst_exp_err__ = EXP_ERR;                                   \
		TST_EXP_FAIL_SILENT_(TST_RET >= 0, SCALL, #SCALL,              \
			&tst_exp_err__, 1, ##__VA_ARGS__);                     \
	} while (0)

/**
 * TST_EXP_EXPR() - Check for expected expression.
 *
 * @EXPR: Expression to be tested.
 * @...: A printf-like parameters.
 *
 * Reports a pass if expression evaluates to non-zero and a fail otherwise.
 *
 * The printf-like parameters can be optionally used to pass a description
 * printed by the pass or fail tst_res() calls. If omitted the expression
 * is converted to a string and used instead.
 */
#define TST_EXP_EXPR(EXPR, ...)                                              \
       tst_res_(__FILE__, __LINE__, (EXPR) ? TPASS : TFAIL, "Expect: "       \
                TST_FMT_(TST_2_(dummy, ##__VA_ARGS__, #EXPR), __VA_ARGS__));

#define TST_EXP_EQ_SILENT_(VAL_A, SVAL_A, VAL_B, SVAL_B, TYPE, PFS) do {       \
	TYPE tst_tmp_a__ = VAL_A;                                              \
	TYPE tst_tmp_b__ = VAL_B;                                              \
                                                                               \
	TST_PASS = 0;                                                          \
                                                                               \
	if (tst_tmp_a__ != tst_tmp_b__) {                                      \
		tst_res_(__FILE__, __LINE__, TFAIL,                            \
			SVAL_A " (" PFS ") != " SVAL_B " (" PFS ")",           \
			tst_tmp_a__, tst_tmp_b__);                             \
	} else {                                                               \
		TST_PASS = 1;                                                  \
	}                                                                      \
} while (0)

/**
 * TST_EXP_EQ_LI() - Compare two long long values.
 *
 * @VAL_A: long long value A.
 * @VAL_B: long long value B.
 *
 * Reports a pass if values are equal and a fail otherwise.
 */
#define TST_EXP_EQ_LI(VAL_A, VAL_B) do {                                       \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, long long, "%lli");   \
								               \
	if (TST_PASS) {                                                        \
		tst_res_(__FILE__, __LINE__, TPASS,                            \
			#VAL_A " == " #VAL_B " (%lli)",                        \
			(long long)VAL_A);                                     \
	}                                                                      \
} while (0)

/**
 * TST_EXP_EQ_LI_SILENT() - Compare two long long values, silent variant.
 *
 * @VAL_A: long long value A.
 * @VAL_B: long long value B.
 *
 * Unlike TST_EXP_EQ_LI() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_EQ_LI_SILENT(VAL_A, VAL_B) \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, long long, "%lli")

/**
 * TST_EXP_EQ_LU() - Compare two unsigned long long values.
 *
 * @VAL_A: unsigned long long value A.
 * @VAL_B: unsigned long long value B.
 *
 * Reports a pass if values are equal and a fail otherwise.
 */
#define TST_EXP_EQ_LU(VAL_A, VAL_B) do {                                       \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, unsigned long long, "%llu"); \
								               \
	if (TST_PASS) {                                                        \
		tst_res_(__FILE__, __LINE__, TPASS,                            \
			#VAL_A " == " #VAL_B " (%llu)",                        \
			(unsigned long long)VAL_A);                            \
	}                                                                      \
} while (0)

/**
 * TST_EXP_EQ_LU_SILENT() - Compare two unsigned long long values, silent variant.
 *
 * @VAL_A: unsigned long long value A.
 * @VAL_B: unsigned long long value B.
 *
 * Unlike TST_EXP_EQ_LU() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_EQ_LU_SILENT(VAL_A, VAL_B) \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, unsigned long long, "%llu")

/**
 * TST_EXP_EQ_SZ() - Compare two unsigned size_t values.
 *
 * @VAL_A: unsigned long long value A.
 * @VAL_B: unsigned long long value B.
 *
 * Reports a pass if values are equal and a fail otherwise.
 */
#define TST_EXP_EQ_SZ(VAL_A, VAL_B) do {                                       \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, size_t, "%zu");       \
								               \
	if (TST_PASS) {                                                        \
		tst_res_(__FILE__, __LINE__, TPASS,                            \
			#VAL_A " == " #VAL_B " (%zu)",                         \
			(size_t)VAL_A);                                        \
	}                                                                      \
} while (0)

/**
 * TST_EXP_EQ_SZ_SILENT() - Compare two unsigned size_t values, silent variant.
 *
 * @VAL_A: unsigned long long value A.
 * @VAL_B: unsigned long long value B.
 *
 * Unlike TST_EXP_EQ_SZ() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_EQ_SZ_SILENT(VAL_A, VAL_B) \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, size_t, "%zu")

/**
 * TST_EXP_EQ_SSZ() - Compare two unsigned ssize_t values.
 *
 * @VAL_A: unsigned long long value A.
 * @VAL_B: unsigned long long value B.
 *
 * Reports a pass if values are equal and a fail otherwise.
 */
#define TST_EXP_EQ_SSZ(VAL_A, VAL_B) do {                                      \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, size_t, "%zi");       \
								               \
	if (TST_PASS) {                                                        \
		tst_res_(__FILE__, __LINE__, TPASS,                            \
			#VAL_A " == " #VAL_B " (%zi)",                         \
			(ssize_t)VAL_A);                                        \
	}                                                                      \
} while (0)

/**
 * TST_EXP_EQ_SSZ_SILENT() - Compare two unsigned ssize_t values, silent variant.
 *
 * @VAL_A: unsigned long long value A.
 * @VAL_B: unsigned long long value B.
 *
 * Unlike TST_EXP_EQ_SSZ() does not print :c:enum:`TPASS <tst_res_flags>` on
 * success, only prints :c:enum:`TFAIL <tst_res_flags>` on failure.
 */
#define TST_EXP_EQ_SSZ_SILENT(VAL_A, VAL_B) \
	TST_EXP_EQ_SILENT_(VAL_A, #VAL_A, VAL_B, #VAL_B, ssize_t, "%zi")

/**
 * TST_EXP_EQ_STR() - Compare two strings.
 *
 * @STR_A: string to compare.
 * @STR_B: string to compare.
 *
 * Reports a pass if strings are equal and a fail otherwise.
 */
#define TST_EXP_EQ_STR(STR_A, STR_B) do {                                      \
	TST_PASS = strcmp(STR_A, STR_B) == 0;                                  \
                                                                               \
	if (TST_PASS) {                                                        \
		tst_res_(__FILE__, __LINE__, TPASS,                            \
			"%s == %s (%s)",                                       \
			#STR_A, #STR_B, STR_B);                                \
	} else {                                                               \
		tst_res_(__FILE__, __LINE__, TFAIL,                            \
			"%s (%s) != %s (%s)",                                  \
			#STR_A, STR_A, #STR_B, STR_B);                         \
	}                                                                      \
} while (0)

/**
 * TST_EXP_EQ_STRN() - Compare two strings, providing length as well.
 *
 * @STR_A: string to compare.
 * @STR_B: string to compare.
 * @LEN: length of the string.
 *
 * Reports a pass if first LEN bytes of strings are equal and a fail otherwise.
 */
#define TST_EXP_EQ_STRN(STR_A, STR_B, LEN) do {                                \
	char str_a_cpy[LEN+1];                                                 \
									       \
	strncpy(str_a_cpy, STR_A, LEN);                                        \
	str_a_cpy[LEN] = 0;                                                    \
									       \
	TST_PASS = strncmp(STR_A, STR_B, LEN) == 0;                            \
									       \
	if (TST_PASS) {                                                        \
		tst_res_(__FILE__, __LINE__, TPASS,                            \
			"%s == %s (%s)",                                       \
			#STR_A, #STR_B, str_a_cpy);                            \
	} else {                                                               \
		char str_b_cpy[LEN+1];                                         \
									       \
		strncpy(str_b_cpy, STR_B, LEN);                                \
		str_b_cpy[LEN] = 0;                                            \
									       \
		tst_res_(__FILE__, __LINE__, TFAIL,                            \
			"%s (%s) != %s (%s)",                                  \
			#STR_A, str_a_cpy, #STR_B, str_b_cpy);                 \
	}                                                                      \
} while (0)

#endif	/* TST_TEST_MACROS_H__ */
