/* foo.h -- interface to the libfoo library
   Copyright (C) 1996-1999 Free Software Foundation, Inc.
   Gordon Matzigkeit <gord@gnu.ai.mit.edu>, 1996
   This file is part of GNU Libtool.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
USA. */

/* Only include this header file once. */
#ifndef _FOO_H_
#define _FOO_H_ 1

/* At some point, cygwin will stop defining __CYGWIN32__, but b19 and
 * earlier do not define __CYGWIN__.  This snippit allows us to check
 * for __CYGWIN32__ reliably for both old and (probable) future releases.
 */
#ifdef __CYGWIN__
#  ifndef __CYGWIN32__
#    define __CYGWIN32__
#  endif
#endif

/* __BEGIN_DECLS should be used at the beginning of your declarations,
   so that C++ compilers don't mangle their names.  Use __END_DECLS at
   the end of C declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

/* LTDL_PARAMS is a macro used to wrap function prototypes, so that compilers
   that don't understand ANSI C prototypes still work, and ANSI C
   compilers can issue warnings about type mismatches. */
#undef LTDL_PARAMS
#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(__CYGWIN32__) || defined(__cplusplus)
# define LT_PARAMS(protos) protos
# define lt_ptr_t     void*
#else
# define LT_PARAMS(protos) ()
# define lt_ptr_t     char*
#endif

#ifdef __CYGWIN32__
#  ifdef LIBFOO_DLL
     /* need some (as yet non-existant) automake magic to tell
      * the object whether the libfoo it will be linked with is
      * a dll or not, ie whether LIBFOO_DLL is defined or not.
      */
#    ifdef _LIBFOO_COMPILATION_
#      define EXTERN __declspec(dllexport)
#    else
#      define EXTERN extern __declspec(dllimport)
#    endif
#  else
#    define EXTERN extern
#  endif
#else
#  define EXTERN extern
#endif

/* Silly constants that the functions return. */
#define HELLO_RET 0xe110
#define FOO_RET 0xf00


/* Declarations.  Note the wonderful use of the above macros. */
__BEGIN_DECLS
int foo LT_PARAMS((void));
int hello LT_PARAMS((void));
EXTERN int nothing;
__END_DECLS

#endif /* !_FOO_H_ */
