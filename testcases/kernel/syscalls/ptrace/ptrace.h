/*
 * ptrace is a fickle beast and each arch sucks in a different way
 */

#ifndef __LTP_PTRACE_H__
#define __LTP_PTRACE_H__

#ifdef HAVE_SYS_PTRACE_H
# include <sys/ptrace.h>
#endif
#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
#endif
#ifdef __ia64__ /* what a pos */
# define ia64_fpreg FU_ia64_fpreg
# define pt_all_user_regs FU_pt_all_user_regs
#endif
#ifdef HAVE_ASM_PTRACE_H
# include <asm/ptrace.h>
#endif
#ifdef HAVE_LINUX_PTRACE_H
# ifndef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
#  include <linux/ptrace.h>
# endif
#endif
#undef FU_ia64_fpreg
#undef FU_pt_all_user_regs

#if defined(HAVE_STRUCT_PT_REGS)
typedef struct pt_regs ptrace_regs;
#elif defined(HAVE_STRUCT_USER_REGS_STRUCT)
typedef struct user_regs_struct ptrace_regs;
#endif

#endif
