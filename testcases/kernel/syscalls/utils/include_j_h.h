/*
 * Crackerjack Project
 *
 * Copyright (C) 2007-2008, Hitachi, Ltd.
 * Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *            Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *            Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Id: include_j_h.h,v 1.2 2009/09/27 17:34:22 subrata_modak Exp $
 *
 */
#ifndef __CJK_SYSCALL_J_H__
#define __CJK_SYSCALL_J_H__

#include <sys/time.h>
#include <sys/resource.h>


#define REG_RESULT_LOG_FP	stdout
#define REG_DETAIL_LOG_FP	stderr


/*
 * RPRINTF : macro to output test result
 */
#define RPRINTF(...)						\
	do {							\
		fprintf(REG_RESULT_LOG_FP, __VA_ARGS__);	\
	} while (0)


/*
 * PRINTF : macro to output detail log
 */
#define PRINTF(...)						\
	do {							\
		fprintf(REG_DETAIL_LOG_FP, __VA_ARGS__);	\
	} while (0)


/*
 * EPRINTF : macro to output error message
 */
#define EPRINTF(...)						\
	do {							\
		fprintf(REG_DETAIL_LOG_FP, __VA_ARGS__);	\
	} while (0)

/*
 * PRINT_XXX : macro to output test result and expect
 */
#define __PRINT_EXPECT(rc_has_range, rc, errno)				\
	do {								\
		if (rc_has_range)					\
			PRINTF("EXPECT: return value(ret)=%s",		\
			       (rc) >= 0 ? "(N >= 0)" : "(N <  0)");	\
		else							\
			PRINTF("EXPECT: return value(ret)=%d", rc);	\
		PRINTF(" errno=%d (%s)", errno, strerror(errno));	\
	} while (0)

#define __PRINT_RESULT(rc_has_range, rc, errno)				\
	do {								\
		if (rc_has_range)					\
			PRINTF("RESULT: return value(ret)=%8d", rc);	\
		else							\
			PRINTF("RESULT: return value(ret)=%d", rc);	\
		PRINTF(" errno=%d (%s)", errno, strerror(errno));	\
	} while (0)

#define PRINT_RESULT(rc_has_range, e_rc, e_errno, r_rc, r_errno)	\
	do {								\
		__PRINT_EXPECT(rc_has_range, e_rc, e_errno);		\
		PRINTF("\n");						\
		__PRINT_RESULT(rc_has_range, r_rc, r_errno);		\
		PRINTF("\n");						\
	} while (0)

#define PRINT_RESULT_EXTRA(rc_has_range, e_rc, e_errno, r_rc, r_errno,	\
			   str, extra_ok)				\
	do {								\
		__PRINT_EXPECT(rc_has_range, e_rc, e_errno);		\
		if ((extra_ok))						\
			PRINTF("\n");					\
		else							\
			PRINTF(", %s=OK\n", str);			\
		__PRINT_RESULT(rc_has_range, r_rc, r_errno);		\
		if ((extra_ok))						\
			PRINTF("\n");					\
		else							\
			PRINTF(", %s=NG\n", str);			\
	} while (0)

#define PRINT_RESULT_CMP(rc_has_range, e_rc, e_errno, r_rc, r_errno, cmp_ok) \
	PRINT_RESULT_EXTRA(rc_has_range, e_rc, e_errno, r_rc, r_errno,	\
			   "r/w check", cmp_ok)


/*
 * Definitions
 */
enum result_val {
	RESULT_OK,
	RESULT_NG
};


/*
 * Prototype
 */
int setup_uid(char *uname);
int setup_euid(char *uname, uid_t *old_uid);
int cleanup_euid(uid_t old_uid);

pid_t create_sig_proc(unsigned long usec, int sig, unsigned count);

int _setup_file(char *testdir, char *fname, char *path, int flags, mode_t mode);
int setup_file(char *testdir, char *fname, char *path);
int cleanup_file(char *path);

int setup_swapfile(char *testdir, char *fname, char *path, size_t size);
int cleanup_swapfile(char *path);

#define QUEUE_NAME	"/test_mqueue"
pid_t create_echo_msg_proc(void);

pid_t get_unexist_pid(void);

#endif /* __CJK_SYSCALL_J_H__ */
