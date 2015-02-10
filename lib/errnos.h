/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009-2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 */

const char *tst_strerrno(int err)
{
	static const struct pair errno_pairs[] = {
		STRPAIR(0, "SUCCESS")
		/* asm-generic/errno-base.h */
		PAIR(EPERM)
		PAIR(ENOENT)
		PAIR(ESRCH)
		PAIR(EINTR)
		PAIR(EIO)
		PAIR(ENXIO)
		PAIR(E2BIG)
		PAIR(ENOEXEC)
		PAIR(EBADF)
		PAIR(ECHILD)
		STRPAIR(EAGAIN, "EAGAIN/EWOULDBLOCK")
		PAIR(ENOMEM)
		PAIR(EACCES)
		PAIR(EFAULT)
		PAIR(ENOTBLK)
		PAIR(EBUSY)
		PAIR(EEXIST)
		PAIR(EXDEV)
		PAIR(ENODEV)
		PAIR(ENOTDIR)
		PAIR(EISDIR)
		PAIR(EINVAL)
		PAIR(ENFILE)
		PAIR(EMFILE)
		PAIR(ENOTTY)
		PAIR(ETXTBSY)
		PAIR(EFBIG)
		PAIR(ENOSPC)
		PAIR(ESPIPE)
		PAIR(EROFS)
		PAIR(EMLINK)
		PAIR(EPIPE)
		PAIR(EDOM)
		PAIR(ERANGE)
		/* asm-generic/errno.h */
		PAIR(EDEADLK)
		PAIR(ENAMETOOLONG)
		PAIR(ENOLCK)
		PAIR(ENOSYS)
		PAIR(ENOTEMPTY)
		PAIR(ELOOP)
		/* EWOULDBLOCK == EAGAIN skipped */
		PAIR(ENOMSG)
		PAIR(EIDRM)
		PAIR(ECHRNG)
		PAIR(EL2NSYNC)
		PAIR(EL3HLT)
		PAIR(EL3RST)
		PAIR(ELNRNG)
		PAIR(EUNATCH)
		PAIR(ENOCSI)
		PAIR(EL2HLT)
		PAIR(EBADE)
		PAIR(EBADR)
		PAIR(EXFULL)
		PAIR(ENOANO)
		PAIR(EBADRQC)
		PAIR(EBADSLT)
		/* EDEADLOCK == EDEADLK skipped */
		PAIR(EBFONT)
		PAIR(ENOSTR)
		PAIR(ENODATA)
		PAIR(ETIME)
		PAIR(ENOSR)
		PAIR(ENONET)
		PAIR(ENOPKG)
		PAIR(EREMOTE)
		PAIR(ENOLINK)
		PAIR(EADV)
		PAIR(ESRMNT)
		PAIR(ECOMM)
		PAIR(EPROTO)
		PAIR(EMULTIHOP)
		PAIR(EDOTDOT)
		PAIR(EBADMSG)
		PAIR(EOVERFLOW)
		PAIR(ENOTUNIQ)
		PAIR(EBADFD)
		PAIR(EREMCHG)
		PAIR(ELIBACC)
		PAIR(ELIBBAD)
		PAIR(ELIBSCN)
		PAIR(ELIBMAX)
		PAIR(ELIBEXEC)
		PAIR(EILSEQ)
		PAIR(ERESTART)
		PAIR(ESTRPIPE)
		PAIR(EUSERS)
		PAIR(ENOTSOCK)
		PAIR(EDESTADDRREQ)
		PAIR(EMSGSIZE)
		PAIR(EPROTOTYPE)
		PAIR(ENOPROTOOPT)
		PAIR(EPROTONOSUPPORT)
		PAIR(ESOCKTNOSUPPORT)
		PAIR(EOPNOTSUPP)
		PAIR(EPFNOSUPPORT)
		PAIR(EAFNOSUPPORT)
		PAIR(EADDRINUSE)
		PAIR(EADDRNOTAVAIL)
		PAIR(ENETDOWN)
		PAIR(ENETUNREACH)
		PAIR(ENETRESET)
		PAIR(ECONNABORTED)
		PAIR(ECONNRESET)
		PAIR(ENOBUFS)
		PAIR(EISCONN)
		PAIR(ENOTCONN)
		PAIR(ESHUTDOWN)
		PAIR(ETOOMANYREFS)
		PAIR(ETIMEDOUT)
		PAIR(ECONNREFUSED)
		PAIR(EHOSTDOWN)
		PAIR(EHOSTUNREACH)
		PAIR(EALREADY)
		PAIR(EINPROGRESS)
		PAIR(ESTALE)
		PAIR(EUCLEAN)
		PAIR(ENOTNAM)
		PAIR(ENAVAIL)
		PAIR(EISNAM)
		PAIR(EREMOTEIO)
		PAIR(EDQUOT)
		PAIR(ENOMEDIUM)
		PAIR(EMEDIUMTYPE)
		PAIR(ECANCELED)
#ifdef ENOKEY
		PAIR(ENOKEY)
#endif
#ifdef EKEYEXPIRED
		PAIR(EKEYEXPIRED)
#endif
#ifdef EKEYREVOKED
		PAIR(EKEYREVOKED)
#endif
#ifdef EKEYREJECTED
		PAIR(EKEYREJECTED)
#endif
#ifdef EOWNERDEAD
		PAIR(EOWNERDEAD)
#endif
#ifdef ENOTRECOVERABLE
		PAIR(ENOTRECOVERABLE)
#endif
#ifdef ERFKILL
		PAIR(ERFKILL)
#endif
#ifdef EHWPOISON
		PAIR(EHWPOISON)
#endif
	};

	PAIR_LOOKUP(errno_pairs, err);
}
