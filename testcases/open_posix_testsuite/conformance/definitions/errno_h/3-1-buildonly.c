/*
 * Following symbols need to be defined in errno.h
 * author:ysun@lnxw.com
 *
 * All #ifdef'ed constants are optional, depending on the spec definition.
 */

#include <errno.h>

int dummy1 = E2BIG;
int dummy2 = EACCES;
int dummy3 = EADDRINUSE;
int dummy4 = EADDRNOTAVAIL;
int dummy5 = EAFNOSUPPORT;
int dummy6 = EAGAIN;
int dummy7 = EALREADY;
int dummy8 = EBADF;
#ifdef EBADMSG
int dummy9 = EBADMSG;
#endif
int dummy10 = EBUSY;
int dummy11 = ECANCELED;
int dummy12 = ECHILD;
int dummy13 = ECONNABORTED;
int dummy14 = ECONNREFUSED;
int dummy15 = ECONNRESET;
int dummy16 = EDEADLK;
int dummy17 = EDESTADDRREQ;
int dummy18 = EDOM;
int dummy19 = EDQUOT;
int dummy20 = EEXIST;
int dummy21 = EFAULT;
int dummy22 = EFBIG;
int dummy23 = EHOSTUNREACH;
int dummy24 = EIDRM;
int dummy25 = EILSEQ;
int dummy26 = EINPROGRESS;
int dummy27 = EINTR;
int dummy28 = EINVAL;
int dummy29 = EIO;
int dummy30 = EISCONN;
int dummy31 = EISDIR;
int dummy32 = ELOOP;
int dummy33 = EMFILE;
int dummy34 = EMLINK;
int dummy35 = EMSGSIZE;
int dummy36 = EMULTIHOP;
int dummy37 = ENAMETOOLONG;
int dummy38 = ENETDOWN;
int dummy39 = ENETRESET;
int dummy40 = ENETUNREACH;
int dummy41 = ENFILE;
int dummy42 = ENOBUFS;
#ifdef ENODATA
int dummy43 = ENODATA;
#endif
int dummy44 = ENODEV;
int dummy45 = ENOENT;
int dummy46 = ENOEXEC;
int dummy47 = ENOLCK;
int dummy48 = ENOLINK;
int dummy49 = ENOMEM;
int dummy50 = ENOMSG;
int dummy51 = ENOPROTOOPT;
int dummy52 = ENOSPC;
#ifdef ENOSR
int dummy53 = ENOSR;
#endif
#ifdef ENOSTR
int dummy54 = ENOSTR;
#endif
int dummy55 = ENOSYS;
int dummy56 = ENOTCONN;
int dummy57 = ENOTDIR;
int dummy58 = ENOTEMPTY;
int dummy59 = ENOTSOCK;
int dummy60 = ENOTSUP;
int dummy61 = ENOTTY;
int dummy62 = ENXIO;
int dummy63 = EOPNOTSUPP;
int dummy64 = EOVERFLOW;
int dummy65 = EPERM;
int dummy66 = EPIPE;
int dummy67 = EPROTO;
int dummy68 = EPROTONOSUPPORT;
int dummy69 = EPROTOTYPE;
int dummy70 = ERANGE;
int dummy71 = EROFS;
int dummy72 = ESPIPE;
int dummy73 = ESRCH;
int dummy74 = ESTALE;
#ifdef ETIME
int dummy75 = ETIME;
#endif
int dummy76 = ETIMEDOUT;
int dummy77 = ETXTBSY;
int dummy78 = EWOULDBLOCK;
int dummy79 = EXDEV;
