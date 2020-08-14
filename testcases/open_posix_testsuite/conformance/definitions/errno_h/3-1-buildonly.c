/*
 * Following symbols need to be defined in errno.h
 * author:ysun@lnxw.com
 *
 * All #ifdef'ed constants are optional, depending on the spec definition.
 */

#include <errno.h>

static int dummy1 = E2BIG;
static int dummy2 = EACCES;
static int dummy3 = EADDRINUSE;
static int dummy4 = EADDRNOTAVAIL;
static int dummy5 = EAFNOSUPPORT;
static int dummy6 = EAGAIN;
static int dummy7 = EALREADY;
static int dummy8 = EBADF;
#ifdef EBADMSG
static int dummy9 = EBADMSG;
#endif
static int dummy10 = EBUSY;
static int dummy11 = ECANCELED;
static int dummy12 = ECHILD;
static int dummy13 = ECONNABORTED;
static int dummy14 = ECONNREFUSED;
static int dummy15 = ECONNRESET;
static int dummy16 = EDEADLK;
static int dummy17 = EDESTADDRREQ;
static int dummy18 = EDOM;
static int dummy19 = EDQUOT;
static int dummy20 = EEXIST;
static int dummy21 = EFAULT;
static int dummy22 = EFBIG;
static int dummy23 = EHOSTUNREACH;
static int dummy24 = EIDRM;
static int dummy25 = EILSEQ;
static int dummy26 = EINPROGRESS;
static int dummy27 = EINTR;
static int dummy28 = EINVAL;
static int dummy29 = EIO;
static int dummy30 = EISCONN;
static int dummy31 = EISDIR;
static int dummy32 = ELOOP;
static int dummy33 = EMFILE;
static int dummy34 = EMLINK;
static int dummy35 = EMSGSIZE;
static int dummy36 = EMULTIHOP;
static int dummy37 = ENAMETOOLONG;
static int dummy38 = ENETDOWN;
static int dummy39 = ENETRESET;
static int dummy40 = ENETUNREACH;
static int dummy41 = ENFILE;
static int dummy42 = ENOBUFS;
#ifdef ENODATA
static int dummy43 = ENODATA;
#endif
static int dummy44 = ENODEV;
static int dummy45 = ENOENT;
static int dummy46 = ENOEXEC;
static int dummy47 = ENOLCK;
static int dummy48 = ENOLINK;
static int dummy49 = ENOMEM;
static int dummy50 = ENOMSG;
static int dummy51 = ENOPROTOOPT;
static int dummy52 = ENOSPC;
#ifdef ENOSR
static int dummy53 = ENOSR;
#endif
#ifdef ENOSTR
static int dummy54 = ENOSTR;
#endif
static int dummy55 = ENOSYS;
static int dummy56 = ENOTCONN;
static int dummy57 = ENOTDIR;
static int dummy58 = ENOTEMPTY;
static int dummy59 = ENOTSOCK;
static int dummy60 = ENOTSUP;
static int dummy61 = ENOTTY;
static int dummy62 = ENXIO;
static int dummy63 = EOPNOTSUPP;
static int dummy64 = EOVERFLOW;
static int dummy65 = EPERM;
static int dummy66 = EPIPE;
static int dummy67 = EPROTO;
static int dummy68 = EPROTONOSUPPORT;
static int dummy69 = EPROTOTYPE;
static int dummy70 = ERANGE;
static int dummy71 = EROFS;
static int dummy72 = ESPIPE;
static int dummy73 = ESRCH;
static int dummy74 = ESTALE;
#ifdef ETIME
static int dummy75 = ETIME;
#endif
static int dummy76 = ETIMEDOUT;
static int dummy77 = ETXTBSY;
static int dummy78 = EWOULDBLOCK;
static int dummy79 = EXDEV;
