/*
 *errno.h shall give positive values for the
 *error number symbolic constants
 *author:ysun@lnxw.com
 */
#include <stdio.h>
#include <errno.h>
#define PTP_PASS        0
#define PTP_FAIL        1
#define PTP_UNRESOLVED  2
#define PTP_NOTINUSE    3
#define PTP_UNSUPPORTED 4
#define PTP_UNTESTED    5
#define PTP_UNINITIATED 6
#define PTP_NORESULT    7

struct unique {
	int value;
	char *name;
} sym[] = { {
E2BIG, "E2BIG"}, {
EACCES, "EACCES"}, {
EADDRINUSE, "EADDRINUSE"}, {
EADDRNOTAVAIL, "EADDRNOTAVAIL"}, {
EAFNOSUPPORT, "EAFNOSUPPORT"}, {
EAGAIN, "EAGAIN"}, {
EALREADY, "EALREADY"}, {
EBADF, "EBADF"}, {
EBADMSG, "EBADMSG"}, {
EBUSY, "EBUSY"}, {
ECANCELED, "ECANCELED"}, {
ECHILD, "ECHILD"}, {
ECONNABORTED, "ECONNABORTED"}, {
ECONNREFUSED, "ECONNREFUSED"}, {
ECONNRESET, "ECONNRESET"}, {
EDEADLK, "EDEADLK"}, {
EDESTADDRREQ, "EDESTADDRREQ"}, {
EDOM, "EDOM"}, {
EDQUOT, "EDQUOT"}, {
EEXIST, "EEXIST"}, {
EFAULT, "EFAULT"}, {
EFBIG, "EFBIG"}, {
EHOSTUNREACH, "EHOSTUNREACH"}, {
EIDRM, "EIDRM"}, {
EILSEQ, "EILSEQ"}, {
EINPROGRESS, "EINPROGRESS"}, {
EINTR, "EINTR"}, {
EINVAL, "EINVAL"}, {
EIO, "EIO"}, {
EISCONN, "EISCONN"}, {
EISDIR, "EISDIR"}, {
ELOOP, "ELOOP"}, {
EMFILE, "EMFILE"}, {
EMLINK, "EMLINK"}, {
EMSGSIZE, "EMSGSIZE"}, {
EMULTIHOP, "EMULTIHOP"}, {
ENAMETOOLONG, "ENAMETOOLONG"}, {
ENETDOWN, "ENETDOWN"}, {
ENETRESET, "ENETRESET"}, {
ENETUNREACH, "ENETUNREACH"}, {
ENFILE, "ENFILE"}, {
ENOBUFS, "ENOBUFS"},
#ifdef ENODATA
{
ENODATA, "ENODATA"},
#endif
{
ENODEV, "ENODEV"}, {
ENOENT, "ENOENT"}, {
ENOEXEC, "ENOEXEC"}, {
ENOLCK, "ENOLCK"}, {
ENOLINK, "ENOLINK"}, {
ENOMEM, "ENOMEM"}, {
ENOMSG, "ENOMSG"}, {
ENOPROTOOPT, "ENOPROTOOPT"}, {
ENOSPC, "ENOSPC"},
#ifdef ENOSR
{
ENOSR, "ENOSR"},
#endif
#ifdef ENOSTR
{
ENOSTR, "ENOSTR"},
#endif
{
ENOSYS, "ENOSYS"}, {
ENOTCONN, "ENOTCONN"}, {
ENOTDIR, "ENOTDIR"}, {
ENOTEMPTY, "ENOTEMPTY"}, {
ENOTSOCK, "ENOTSOCK"}, {
ENOTSUP, "ENOTSUP"}, {
ENOTTY, "ENOTTY"}, {
ENXIO, "ENXIO"}, {
EOPNOTSUPP, "EOPNOTSUPP"}, {
EOVERFLOW, "EOVERFLOW"}, {
EPERM, "EPERM"}, {
EPIPE, "EPIPE"}, {
EPROTO, "EPROTO"}, {
EPROTONOSUPPORT, "EPROTONOSUPPORT"}, {
EPROTOTYPE, "EPROTOTYPE"}, {
ERANGE, "ERANGE"}, {
EROFS, "EROFS"}, {
EWOULDBLOCK, "EWOULDBLOCK"}, {
EXDEV, "EXDEV"}, {
0, 0}
};

int main()
{
	struct unique *tst = sym;
	int ret = PTP_PASS;
	while (tst->name) {
		if (tst->value < 0) {
			printf("Value of symbol %s is less than zero\n",
			       tst->name);
			ret = PTP_FAIL;
		}
		tst++;
	}
	return ret;
}
