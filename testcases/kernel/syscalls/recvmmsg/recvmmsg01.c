/*{
 * Copyright (c) Google LLC, 2018
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"

/*
 *	Comment out to the #define
 *		#define	RECVMMSG_USE_SCTP
 *	to test without SCTP even if it is provided configured in your system.
 *
 *	Note that HAVE_NETINET_SCTP_H comes from the:
 *		#include "tst_test.h"
 *	so don't mmve this code above that include.
 */

#ifdef HAVE_NETINET_SCTP_H
#define	RECVMMSG_USE_SCTP
#include <netinet/sctp.h>
#endif

/*}
 *{	Contants.
 */

enum { GUARD_CHAR = '+' };		/* guard character to check overwrites*/
enum { INVALID_SYSCALL_VALUE = -2 };	/* syscalls values are all >= -1 */
enum { TEST_IP = 0x7F000001 };
enum { TEST_PORT = 6789 };
enum { SENDMSG_SLEEP_MS = 10 };

/*
 * Must be disjoint with errno values and non-zero, value returned by tests
 * that do multiple recvmsg() calls internally, or that have a non-zero flag,
 * or that require special treatment (sender needs to sleep).  Those tests
 * can not be bundled with other tests into a larger recvmmsg(2) multi-
 * receive message test, some can only be bundled with their own kind.
 */

enum non_error_errno {
	TEST_HAS_MULTIPLE_RECVMSG = -1,		/* can't be bundled at all */
	TEST_CLOEXEC_FLAG = -2,			/* bundle these together */
	TEST_NEEDS_SLEEP_IN_SENDER = -3,	/* bundle these together */
};

/*}
 *{	Typedefs.
 */

typedef struct timeval		timeval_t;
typedef struct timespec		timespec_t;
typedef struct iovec		iovec_t;
typedef struct msghdr		msghdr_t;
typedef struct mmsghdr		mmsghdr_t;
typedef struct sockaddr		sockaddr_t;
typedef struct sockaddr_in	sockaddr_in_t;
typedef struct sctp_initmsg	sctp_initmsg_t;
typedef struct cmsghdr		cmsghdr_t;
typedef struct call_s		call_t;

typedef void *(*thread_start_t)(void *arg);

typedef struct {
	cmsghdr_t	cmf_cmsg;
	int		cmf_fdspace;  /* padding in cmf_cmsg implies fd might */
				      /* not be here, might be in cmf_cmsg! */
} ctlmsgfd_t;

typedef struct {
	pthread_t	 tc_thr;
} thread_call_t;

/*
 * sendmsg(2) call to be sent individually or bundled up with other such
 * calls with sendmmsg(2) by copying them into an mmsghdr_t array, the
 * sending is done by a pthread_create(3) created thread
 */

typedef struct {
	thread_call_t	 smc_thread_call;  /* must be first: inherits from */
	int		 smc_sockfd;
	msghdr_t	*smc_msg;
	int		 smc_flags;
	ssize_t		 smc_value;
	int		 smc_errno;
} sendmsg_call_t;

/*
 * sendmmsg(2) call, will be done as a single sendmmsg(2) call or vlen
 * individual sendmsg(2) calls
 */

typedef struct {
	thread_call_t	 smmc_thread_call;  /*  must be first: inherits from */
	int		 smmc_sockfd;
	unsigned	 smmc_vlen;
	mmsghdr_t	*smmc_smm;
	int		 smmc_flags;
	bool		 smmc_call_sendmmsg;
	bool		 smmc_sender_sleeps;
	int		 smmc_value;
	int		 smmc_errno;
} sendmmsg_call_t;

/*
 * recvmsg(2) call to be received individually or bundled up with other
 * such calls with recvmmsg(2) by copying them into an mmsghdr_t array
 */

typedef struct {
	ssize_t		 rmc_value;
	int		 rmc_errno;
	msghdr_t	*rmc_msg;
	int		 rmc_flags;
} recvmsg_call_t;

/*
 * client server connection
 *
 * A con_t abstracts a connection between a client and a server, it is used
 * two different ways:
 *
 * con_call_vec == NULL && con_call_n == 0 && con_call_sendmmsg == false
 *
 * - Individual tests encapsulated in a single test function, these are
 *   usually just one sendmsg(2) and one or maybe two sequential recvmsg(2)
 *   calls, for example to peek then get the message or to test non-blocking
 *   then blocking recvmsg, etc.
 *
 * con_call_vec != NULL && con_call_n >= 1
 *
 * - Bundled unrelated tests each encapsulated in their own test function,
 *   all of these tests have to be implemented with a 1 send and 1 recveive,
 *   so that they can all run with a multi-receive message, recvmmsg(2), call.
 *   In that case the con_t is used to hide the bundling of the tests from
 *   each other and the tests, unsupectingly, pass the test vector via the
 *   con_call_vec and con_call_n members.  Note that con_call_n == 1 is used
 *   as a degenerate case to test recvmmsg(2) with a single message. Note
 *   when recvmmsg(2) is used the messages can be sent individually or in
 *   a single sendmmsg(2) for testing multi-message sending. Both ways of
 *   sending are done controlled by con_call_sendmmsg == true to indicate
 *   used of sendmmsg(2), otherwise multiple sendmsg(2) are used.
 */

typedef struct {
	int		  con_client;
	int		  con_server;
	int		  con_type;
	sockaddr_in_t	  con_server_sin;
	bool		  con_call_sendmmsg;
	bool		  con_sender_sleeps;
	bool		  con_waitforone;
	bool		  con_timeout;
	size_t		  con_call_end;	/* index of last+1 filled */
	size_t		  con_call_n;
	call_t		**con_call_vec;	/* points to array of con_call_n */
					/* call_t pointers to be used in */
					/* a recvmmsg(2) multi-message test */
	sendmsg_call_t	**con_smc_vec;
	recvmsg_call_t	**con_rmc_vec;
} con_t;

typedef int (*calltest_t)(call_t *callp, con_t *conp, int tn);
typedef void (*callconinit_t)(call_t *callp, con_t *conp);

struct call_s {
	calltest_t	call_test;
	callconinit_t	call_con_init;
	int		call_errno;	/* set on first run */
};

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
	size_t	call_niov;
} call_receive_iovec_boundary_checks_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
	size_t	call_niov;
} call_receive_iovec_boundary_checks_peeking_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
	int	call_cloexec_flag;
	bool	call_some_data;
	ssize_t	call_controllen_delta;
} call_receive_file_descriptor_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
	int	call_type;
} call_message_too_long_to_fit_might_discard_bytes_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
} call_receive_waits_for_message_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
} call_receiver_doesnt_wait_for_messages_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
} call_receive_returns_what_is_available_t;

typedef struct {
	call_t	call_base;		/* must be first: inherits call_t */
} call_empty_datagram_received_as_empty_datagram_t;

/*}
 *{	Prototypes.
 *
 *	These functions are all static because that is what the LTP coding
 *	convention likes, doing this makes no different other than making
 *	the code harder to debug. To be able to make them non-static for
 *	easier debugging (for example so their names are in the symbol table
 *	for core dump debugging, etc) STATIC is used instead, which can be
 *	easily changed to remove the static declarator which gets in the way
 *	of putting symbols in the symbol table.
 */

// #define STATIC
#define	STATIC static

STATIC void tests_setup(void);
STATIC int tests_select(callconinit_t cci, call_t *tests_src[],
			call_t *tests_dest[], size_t n, int error);

STATIC void tests_run(void);
STATIC void tests_run_individually(void);
STATIC void tests_run_all_socketpair(void);
STATIC void tests_run_all_ocloexec_flag(void);
STATIC void tests_run_all_sock_sepacket(void);
#ifdef RECVMMSG_USE_SCTP
STATIC void tests_run_all_sock_sepacket(void);
STATIC void tests_run_all_sock_sepacket_with_n_repeat(void);
#endif

STATIC void sleep_ms(long ms);

STATIC void thread_call(thread_call_t *tcp, thread_start_t func);
STATIC void thread_join(thread_call_t *tcp);

STATIC int test_number_get(void);

STATIC int subtest_number(void);
STATIC void subtest_nest(void);
STATIC void subtest_unnest(void);
STATIC bool subtest_show(void);

STATIC bool mem_is_equal(void *a, void *b, size_t size);
STATIC bool mem_is_zero(void *a, size_t size);

STATIC void sockaddr_in_init(sockaddr_in_t *sinp, sa_family_t family,
			     in_port_t port, uint32_t ip);
STATIC void sockaddr_in_init_to_zero(sockaddr_in_t *sinp);

STATIC char *sock_type_to_str(int type);

STATIC int errno_value_is_error(int e);

STATIC size_t iovec_max(void);
STATIC void iovec_init(iovec_t *iovecp, void *base, size_t len);

STATIC void msghdr_init(msghdr_t *msgp, sockaddr_in_t *sinp,
			iovec_t *iovecp, size_t iovlen, int flags);
STATIC void msghdr_init_with_control(msghdr_t *msgp, sockaddr_in_t *sinp,
				     iovec_t *iovecp, size_t iovlen,
				     void *control, size_t controllen,
				     int flags);

STATIC void mmsghdr_init(mmsghdr_t *mmsgp, msghdr_t *msgp);

STATIC void recvmsg_call_init(recvmsg_call_t *rmcp,
			      msghdr_t *msgp, int flags);

STATIC void sendmsg_call_init(sendmsg_call_t *smcp, int sockfd,
			      msghdr_t *msgp, int flags);
STATIC void *sendmsg_call(sendmsg_call_t *smcp);
STATIC void *sendmsg_call_after_sleep(sendmsg_call_t *smcp);

STATIC void sendmmsg_init(sendmmsg_call_t *smmcp, int sockfd, unsigned vlen,
			  mmsghdr_t *smm, int flags, bool call_sendmmsg,
			  bool sender_sleeps);

STATIC void ctlmsgfd_init(ctlmsgfd_t *cmfp, int fd);
STATIC void ctlmsgfd_init_to_zero(ctlmsgfd_t *cmfp);
STATIC int ctlmsgfd_get_fd(ctlmsgfd_t *cmfp);

STATIC void con_init(con_t *conp, int client, int server,
		     int type, sockaddr_in_t *sinp);
STATIC void con_init_base(con_t *conp, int client, int server,
			  int type, sockaddr_in_t *sinp);
STATIC void con_make_vectored(con_t *conp, bool sendmulti,
			      size_t n, call_t **call_vec,
			      sendmsg_call_t **smc_vec,
			      recvmsg_call_t **rmc_vec);
STATIC void con_make_waitforone_tests(con_t *conp);
STATIC void con_make_timeout_tests(con_t *conp);
STATIC void con_init_with_type(con_t *conp, int type);
STATIC void con_init_socketpair(con_t *conp);
STATIC void con_init_seqpacket(con_t *conp);
STATIC void con_init_dgram(con_t *conp);
STATIC void con_deinit(con_t *conp);
STATIC void con_sendmmsg_recvmmsg(con_t *conp, size_t n,
				  int sflags, mmsghdr_t *smm,
				  int rflags, mmsghdr_t *rmm);
STATIC void con_do_multi_send_recv(con_t *conp);
STATIC void con_add_send_recv_calls_vec(con_t *conp, sendmsg_call_t *smcp,
					recvmsg_call_t *rmcp,
					bool sender_sleeps);
STATIC void con_add_send_recv_calls(con_t *conp, sendmsg_call_t *smcp,
				    recvmsg_call_t *rmcp, bool sender_sleeps);

STATIC int call_go(call_t *callp, con_t *conp);
STATIC void call_do_con_init_socketpair(call_t *callp, con_t *conp);
STATIC void call_do_con_init_seqpacket(call_t *callp, con_t *conp);
STATIC void call_do_con_init_dgram(call_t *callp, con_t *conp);
STATIC void call_base_init(call_t *callp, callconinit_t coninit,
			   calltest_t test);

STATIC void call_receive_iovec_boundary_checks_init(
		call_receive_iovec_boundary_checks_t *callp,
		size_t niov);
STATIC int call_receive_iovec_boundary_checks(
		call_receive_iovec_boundary_checks_t *callp,
		con_t *conp, int tn);

STATIC void call_receive_iovec_boundary_checks_peeking_init(
		call_receive_iovec_boundary_checks_peeking_t *callp,
		size_t niov);
STATIC int call_receive_iovec_boundary_checks_peeking(
		call_receive_iovec_boundary_checks_peeking_t *callp,
		con_t *conp, int tn);

STATIC void call_receive_file_descriptor_init(
		call_receive_file_descriptor_t *callp,
		int cloexec_flag, bool some_data, ssize_t controllen_delta);
STATIC int call_receive_file_descriptor(
		call_receive_file_descriptor_t *callp, con_t *conp, int tn);

STATIC void call_msgtoolong_do_con_init_from_type(
		call_message_too_long_to_fit_might_discard_bytes_t *callp,
		con_t *conp);
STATIC void call_message_too_long_to_fit_might_discard_bytes_init(
		call_message_too_long_to_fit_might_discard_bytes_t *callp,
		int type);
STATIC int call_message_too_long_to_fit_might_discard_bytes(
		call_message_too_long_to_fit_might_discard_bytes_t *callp,
		con_t *conp, int tn);

STATIC void call_empty_datagram_received_as_empty_datagram_init(
		call_empty_datagram_received_as_empty_datagram_t *callp);
STATIC int call_empty_datagram_received_as_empty_datagram(
		call_empty_datagram_received_as_empty_datagram_t *callp,
		con_t *conp, int tn);

STATIC void call_receive_waits_for_message_init(
		call_receive_waits_for_message_t *callp);
STATIC int call_receive_waits_for_message(
		call_receive_waits_for_message_t *callp, con_t *conp, int tn);

STATIC void call_receiver_doesnt_wait_for_messages_init(
		call_receiver_doesnt_wait_for_messages_t *callp);
STATIC int call_receiver_doesnt_wait_for_messages(
		call_receiver_doesnt_wait_for_messages_t *callp,
		con_t *conp, int tn);

STATIC void call_receive_returns_what_is_available_init(
		call_receive_returns_what_is_available_t *callp);
STATIC int call_receive_returns_what_is_available(
		call_receive_returns_what_is_available_t *callp,
		con_t *conp, int tn);

STATIC int con_receive_iovec_boundary_checks(con_t *conp, int tn, size_t niov);
STATIC int con_receive_iovec_boundary_checks_peeking(con_t *conp, int tn,
						     size_t niov);
STATIC int con_receive_file_descriptor(con_t *conp, int tn, int cloexec_flag,
				       bool some_data,
				       ssize_t controllen_delta);
STATIC int con_message_too_long_to_fit_might_discard_bytes(con_t *conp, int tn,
							   int type);
STATIC int con_receive_waits_for_message(con_t *conp, int tn);
STATIC int con_receiver_doesnt_wait_for_messages(con_t *conp, int tn);
STATIC int con_receive_returns_what_is_available(con_t *conp, int tn);
STATIC int con_empty_datagram_received_as_empty_datagram(con_t *conp, int tn);

/*}
 *{	Misc.
 */

char *prog_name;
#define	unused(x) ((void)(x))

/*
 * Error reporting and exiting.
 */

int test_verbose = 0;	/* 1 a bit verbose, 2 more verbose */
int ensure_failures;

/*
 * ensure() tests something that shouldn't fail, doesn't abort like assert()
 *
 * Test cases use ensure(expr) to indicate that an expression is expected
 * to be true.  When the test cases are run in a verbose manner, the ensure
 * uses produce a trace of the conditions being tested, making it easier
 * to understand what the test cases are verifying.
 * 
 * Some test cases, depending on their arguments, test for the same condition
 * and its opposite.  For easier reading of the verbose output extra ensure()
 * uses are made to cause useful output to be produced, for example on both
 * branches of an if-else that has already tested the condition. In these cases
 * the ensure() also serves as a trace. This is not sloppy coding, its done
 * purposely, don't delete such code. For example:
 * 
 * 	if (server_recv.msg_flags & MSG_EOR) {
 * 		ensure(server_recv.msg_flags & MSG_EOR);
 * 		...
 * 	} else {
 * 		ensure(! (server_recv.msg_flags & MSG_EOR));
 * 		...
 * 	}
 * 
 * Messages are not shown unless an expected condition is not true, but if
 * the level of verbosity is high enough, then the non-failing ensure()
 * expressions are shown too, this is just for debug-ability of the tests,
 * in this case the ensure() serve as a trace.
 *
 * A test, made out of multiple ensure() calls is expected not to change the
 * value of ensure_failures.
 */

#define ensure(test_number, expr)					    \
	((expr) ? (test_verbose < 2 ? (void)1 :				    \
		   tst_res(TINFO, "%s(): true: %s", __FUNCTION__, #expr))   \
		: (++ensure_failures,					    \
		   tst_res(TINFO, "test_number = %d: %s(): false: %s",	    \
			   test_number, __FUNCTION__, #expr)))

/*}
 *{	Test functions.
 *
 *	The lowest level test functions are organized as standalone test
 *	functions that can check some aspect of sending and receiving messages
 *	works per the system call documentation.
 *
 *	These lowest level test functions are written as straight line code
 *	that sets up a message to be send, hands the message to a thread,
 *	to send it and the receiver proceeds to receive the message.
 *
 *	Some of these test cases involve multiple sends and receives to test
 *	various conditions, but most involve a single send and receive.
 *
 *	To be able to test multi-send and multi-receive sequences, the lowest
 *	level test functions are combined into sendmmsg(2) and recvmmsg(2)
 *	system calls. The underlying lowest level test functions don't know
 *	about this, but each one of them does the setup logical sending (which
 *	in this case is postponed to be combined into a multi-message send)
 *	and the logical receiving (which in this case works on the proper
 *	message received from a multi-message receive operation).
 *
 *	Some lowest level test functions can be combined with others, some
 *	can't, for example because they use a different type of connection,
 *	for example a socketpair() vs an SCTP/IP connection. Or because they
 *	require different flags, and the send message flag is shared by all
 *	the messages being sent, it is not per message, so they can only
 *	be aggregated into a multi-message operation if their flags don't
 *	conflict.
 *
 *	The test selection and combination functions can be found by
 *	searching for the callers of: tests_select()
 */

/*
 *  Various boundary tests related to use of more than one iovec
 */

STATIC int con_receive_iovec_boundary_checks(con_t *conp, int tn, size_t niov)
{
	size_t iov_max = iovec_max();
	int salt = tn + subtest_number();	/* salt test data */

	int client_data[niov];
	iovec_t client_iov[niov];
	size_t i;
	for (i = 0; i < niov; ++i) {
		client_data[i] = i + salt;
		iovec_init(&client_iov[i], &client_data[i],
			   sizeof(client_data[0]));
	}

	int server_data[niov];
	iovec_t server_iov[niov];
	for (i = 0; i < niov; ++i) {
		server_data[i] = 0;
		iovec_init(&server_iov[i], &server_data[i],
			   sizeof(server_data[0]));
	}

	msghdr_t client_send;
	msghdr_init(&client_send, NULL, client_iov, niov, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	msghdr_t server_recv;
	msghdr_init(&server_recv, NULL, server_iov, niov, 0);

	recvmsg_call_t rmc;
	recvmsg_call_init(&rmc, &server_recv, 0);

	con_add_send_recv_calls(conp, &smc, &rmc, false);

	ssize_t received = rmc.rmc_value;
	if (niov > iov_max) {
		ensure(tn, smc.smc_value == -1);
		ensure(tn, smc.smc_errno == EMSGSIZE);
		ensure(tn, received == -1);
		ensure(tn, rmc.rmc_errno == EMSGSIZE);
		ensure(tn, mem_is_zero(server_data, sizeof(server_data)));
		return errno;
	}

	if (smc.smc_value < 0)
		tst_res(TBROK, "sendmsg");
	if (received < 0)
		tst_res(TBROK, "recmsg or recvmmsg");
	ensure(tn, (size_t) received == sizeof(client_data));
	ensure(tn, mem_is_equal(client_data, server_data, received));
	return 0;
}

/*
 * Various boundary tests related to use of more than one iovec,
 * peek at the data first, then receive it
 */

STATIC int con_receive_iovec_boundary_checks_peeking(con_t *conp, int tn,
						     size_t niov)
{
	size_t iov_max = iovec_max();

	int client_data[niov];
	iovec_t client_iov[niov];
	size_t i;
	for (i = 0; i < niov; ++i) {
		client_data[i] = i;
		iovec_init(&client_iov[i], &client_data[i],
			   sizeof(client_data[0]));
	}

	int server_data[niov];
	iovec_t server_iov[niov];
	for (i = 0; i < niov; ++i) {
		server_data[i] = 0;
		iovec_init(&server_iov[i], &server_data[i],
			   sizeof(server_data[0]));
	}

	msghdr_t client_send;
	msghdr_init(&client_send, NULL, client_iov, niov, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	msghdr_t server_recv;
	msghdr_init(&server_recv, NULL, server_iov, niov, 0);

	thread_call(&smc.smc_thread_call, (thread_start_t) sendmsg_call);
	ssize_t received = recvmsg(conp->con_server, &server_recv, MSG_PEEK);
	thread_join(&smc.smc_thread_call);

	if (niov > iov_max) {
		ensure(tn, smc.smc_value == -1);
		ensure(tn, smc.smc_errno == EMSGSIZE);
		ensure(tn, received == -1);
		ensure(tn, errno == EMSGSIZE);
		ensure(tn, mem_is_zero(server_data, sizeof(server_data)));
	} else {
		if (smc.smc_value < 0)
			tst_res(TBROK, "sendmsg");
		if (received < 0)
			tst_res(TBROK, "recvmsg");
		ensure(tn, (size_t) received == sizeof(client_data));
		ensure(tn, mem_is_equal(client_data, server_data, received));
	}

	received = recvmsg(conp->con_server, &server_recv, 0);
	if (niov > iov_max) {
		ensure(tn, smc.smc_value == -1);
		ensure(tn, smc.smc_errno == EMSGSIZE);
		ensure(tn, received == -1);
		ensure(tn, errno == EMSGSIZE);
		ensure(tn, mem_is_zero(server_data, sizeof(server_data)));
		return errno;
	}

	if (smc.smc_value < 0)
		tst_res(TBROK, "sendmsg");
	if (received < 0)
		tst_res(TBROK, "recvmsg");
	ensure(tn, (size_t) received == sizeof(client_data));
	ensure(tn, mem_is_equal(client_data, server_data, received));
	return TEST_HAS_MULTIPLE_RECVMSG;
}

/*
 * Test that file descriptor is received over a unix domain socket.
 * Variations to test the close on exec flag, whether some amount of
 * data is to be sent as regular data, and missing bytes of space for
 * the control message to receive the file descriptor.  The missing
 * bytes are expressed as a (signed) ssize_t controllen_delta.  Passing
 * in cmsghdr_t requires a negative delta to be small enough so as to
 * actually cause trancation of the space for an int sized file descriptor,
 * a negative controllen_delta must be <= -(ssize_t)(sizeof(size_t)).
 */

STATIC int con_receive_file_descriptor(con_t *conp, int tn, int cloexec_flag,
				       bool some_data,
				       ssize_t controllen_delta)
{
	assert(controllen_delta >= 0 ||
	       controllen_delta <= -(ssize_t) (sizeof(size_t)));
	assert(cloexec_flag == 0 || cloexec_flag == MSG_CMSG_CLOEXEC);

	int pipefd[2];
	SAFE_PIPE(pipefd);	/* something to send that's easy to check */
	int pipe_read = pipefd[0];
	int pipe_write = pipefd[1];

	ctlmsgfd_t client_ctlmsgfd;
	ctlmsgfd_init(&client_ctlmsgfd, pipe_write);

	iovec_t client_iov;
	char server_data = 0;
	iovec_init(&client_iov, "m", 1);
	iovec_t server_iov;
	iovec_init(&server_iov, &server_data, 1);

	iovec_t *client_iovp = NULL;
	iovec_t *server_iovp = NULL;
	size_t client_iovlen = 0;
	size_t server_iovlen = 0;
	size_t expected = 0;
	if (some_data) {
		client_iovp = &client_iov;
		server_iovp = &server_iov;
		client_iovlen = 1;
		server_iovlen = 1;
		expected = 1;
	}

	msghdr_t client_send;
	msghdr_init_with_control(&client_send, NULL, client_iovp, client_iovlen,
				&client_ctlmsgfd, sizeof(client_ctlmsgfd), 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	ctlmsgfd_t server_ctlmsgfd;
	ctlmsgfd_init_to_zero(&server_ctlmsgfd);

	msghdr_t server_recv;
	msghdr_init_with_control(&server_recv, NULL, server_iovp, server_iovlen,
				&server_ctlmsgfd,
				(size_t) ((ssize_t) sizeof(server_ctlmsgfd)
					  + controllen_delta),
				0);

	recvmsg_call_t rmc;
	recvmsg_call_init(&rmc, &server_recv, cloexec_flag);

	con_add_send_recv_calls(conp, &smc, &rmc, false);

	ssize_t received = rmc.rmc_value;
	if (received < 0)
		tst_res(TBROK, "recvmsg or recvmmsg");

	ensure(tn, (size_t) received == expected);
	if (some_data)
		ensure(tn, server_data == 'm');

	if (controllen_delta < 0)
		ensure(tn, server_recv.msg_flags & MSG_CTRUNC);
	else {
		ensure(tn, ! (server_recv.msg_flags & MSG_CTRUNC));
		ensure(tn, server_ctlmsgfd.cmf_cmsg.cmsg_len >=
		       sizeof(ctlmsgfd_t));
		int received_fd = ctlmsgfd_get_fd(&server_ctlmsgfd);
		ensure(tn, received_fd > 0);

		ssize_t nwrote = SAFE_WRITE(0, received_fd, "p", 1);
		ensure(tn, nwrote == 1);

		char c;
		ssize_t nread = SAFE_READ(0, pipe_read, &c, 1);
		ensure(tn, nread == 1);
		ensure(tn, c == 'p');

		int fdflags = SAFE_FCNTL(received_fd, F_GETFD, 0);
		if (cloexec_flag)
			ensure(tn, fdflags & FD_CLOEXEC);
		else
			ensure(tn, ! (fdflags & FD_CLOEXEC));
		close(received_fd);
	}

	close(pipe_read);
	close(pipe_write);
	if (cloexec_flag)
		return TEST_CLOEXEC_FLAG;	/* can not mix flags */
	return 0;
}

/*
 * To be able to test this specification in the recvmsg(2) man page:
 *
 *	"If a message is too long to fit in the supplied buffer,
 *	 excess bytes may be discarded depending on the type of
 *	 socket the message is received from."
 *
 * The udp(7) protocol discards the excess bytes, the sctp(7) does not.
 * This test is meant to be used with both, tests for the excess bytes being
 * discarded or for MSG_EOR being set together with the non-discarded bytes.
 * The test sends a 16 byte message, but receives it with an 8 byte buffer,
 * a second recvmsg(2) is done to see if the next 8 bytes are of a 2nd
 * identical 16 byte packet sent or if they belong to the first packet.
 */

STATIC int con_message_too_long_to_fit_might_discard_bytes(con_t *conp, int tn,
							   int type)
{
	assert(type == SOCK_SEQPACKET || type == SOCK_DGRAM);

	size_t total = 16;
	assert(total % 2 == 0);				/* must be even */
	size_t half = total / 2;

	iovec_t client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	assert(strlen(client_iov.iov_base) == total);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[half + 1];	/* client sends total, receive half */
	memset(&server_data, 0, half);	/* 1 byte of space for GUARD_CHAR */
	server_data[half] = GUARD_CHAR;
	iovec_t server_iov;
	iovec_init(&server_iov, server_data, half);

	sockaddr_in_t client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	msghdr_t server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	thread_call(&smc.smc_thread_call, (thread_start_t) sendmsg_call);
	ssize_t received = recvmsg(conp->con_server, &server_recv, 0);
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		tst_res(TBROK, "recvmsg");

	ensure(tn, (size_t) received == half);
	ensure(tn, server_data[half] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, half));
	if (type == SOCK_SEQPACKET)
		ensure(tn, type == SOCK_SEQPACKET &&
		       server_recv.msg_flags == 0);
	else
		ensure(tn, type == SOCK_DGRAM &&
		       server_recv.msg_flags & MSG_TRUNC);

	/*
	 * send the same data in a second from client, should receive
	 * the same data, not the second half of the first message, if
	 * this test passes, then the excess bytes are being discarded,
	 * but if it receives the second half, then they are not being
	 * discarded and MSG_EOR indicates the end of the first record
	 */

	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	thread_call(&smc.smc_thread_call, (thread_start_t) sendmsg_call);
	received = recvmsg(conp->con_server, &server_recv, MSG_TRUNC);
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		tst_res(TBROK, "recvmsg");

	/*
	 * MSG_TRUNC in this recvmsg() asks for real size of the datagram,
	 * not the read size, for SOCK_DGRAM this should be total not half
	 * the GUARD_CHAR check below ensures that no more than half were
	 * read, and the check on the read bytes ensures half were returned
	 *
	 *	"MSG_TRUNC"
	 *	"For raw (AF_PACKET), Internet datagram, netlink, and UNIX
	 *	 datagram sockets: return the real length of the packet or
	 *	 datagram, even when it was longer than the passed buffer."
	 *							-- recvmsg(2)
	 */

	if (type == SOCK_DGRAM)
		ensure(tn, (size_t) received == total);
	else
		ensure(tn, (size_t) received == half);
	ensure(tn, server_data[half] == GUARD_CHAR);
	if (server_recv.msg_flags & MSG_EOR) {
		ensure(tn, server_recv.msg_flags & MSG_EOR);
		ensure(tn, mem_is_equal(server_data,
					client_iov.iov_base + half, half));
	} else {
		ensure(tn, ! (server_recv.msg_flags & MSG_EOR));
		ensure(tn,
		       mem_is_equal(server_data, client_iov.iov_base, half));
	}
	return TEST_HAS_MULTIPLE_RECVMSG;
}

/*
 * recvmsg(2):
 *
 *	"If no messages are available at the socket, the
 *	 receive calls wait for a message to arrive"
 *
 * The sending thread waits a second prior to sending to see if the
 * recvmsg(2) indeed waits for the message to arrive.
 */

STATIC int con_receive_waits_for_message(con_t *conp, int tn)
{
	size_t total = 16;
	iovec_t client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	assert(strlen(client_iov.iov_base) == total);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[total + 1];	
	memset(&server_data, 0, total);	/* 1 byte of space for GUARD_CHAR */
	server_data[total] = GUARD_CHAR;
	iovec_t server_iov;
	iovec_init(&server_iov, server_data, total);

	sockaddr_in_t client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	msghdr_t server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	recvmsg_call_t rmc;
	recvmsg_call_init(&rmc, &server_recv, 0);

	con_add_send_recv_calls(conp, &smc, &rmc, true);

	ssize_t received = rmc.rmc_value;
	if (received < 0)
		tst_res(TBROK, "recvmsg or recvmmsg");

	ensure(tn, (size_t) received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	return TEST_NEEDS_SLEEP_IN_SENDER;
}

/*
 * recvmsg(2):
 *
 *	"If no messages are available at the socket, the
 *	 receive calls wait for a message to arrive, unless 
 *	 the socket is nonblocking (see fcntl(2)), in which
 *	 case the value -1 is returned and the external variable 
 *	 errno is set to EAGAIN or EWOULDBLOCK."
 *
 * If a message is never sent, the receiver should not block.
 */

STATIC int con_receiver_doesnt_wait_for_messages(con_t *conp, int tn)
{
	size_t total = 16;
	iovec_t client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	assert(strlen(client_iov.iov_base) == total);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[total + 1];	
	memset(&server_data, 0, total);	/* 1 byte of space for GUARD_CHAR */
	server_data[total] = GUARD_CHAR;
	iovec_t server_iov;
	iovec_init(&server_iov, server_data, total);

	sockaddr_in_t client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	msghdr_t server_recv;
	memset(&server_recv, 0, sizeof(server_recv));
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	thread_call(&smc.smc_thread_call,
		    (thread_start_t) sendmsg_call_after_sleep);
	ssize_t received = recvmsg(conp->con_server,
				   &server_recv, MSG_DONTWAIT);
	ensure(tn, received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
	received = recvmsg(conp->con_server, &server_recv, 0);	/* now wait */
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		tst_res(TBROK, "recvmsg");

	ensure(tn, (size_t) received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	return TEST_HAS_MULTIPLE_RECVMSG;
}

/*
 * recvmsg(2):
 *
 *	"The receive calls normally return any data available,
 *	 up to the requested amount, rather than waiting for
 *	 receipt of the full amount requested."
 */

STATIC int con_receive_returns_what_is_available(con_t *conp, int tn)
{
	size_t total = 8;
	iovec_t client_iov;
	iovec_init(&client_iov, "01234567", total);
	assert(strlen(client_iov.iov_base) == total);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	size_t twice = 2 * total;
	char server_data[twice + 1];	/* client sends total, want twice */
	memset(&server_data, 0, twice);	/* 1 byte of space for GUARD_CHAR */
	server_data[total] = GUARD_CHAR;
	server_data[twice] = GUARD_CHAR;	/* set two guards */
	iovec_t server_iov;
	iovec_init(&server_iov, server_data, twice);

	sockaddr_in_t client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	msghdr_t server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	recvmsg_call_t rmc;
	recvmsg_call_init(&rmc, &server_recv, 0);

	con_add_send_recv_calls(conp, &smc, &rmc, false);

	ssize_t received = rmc.rmc_value;
	if (received < 0)
		tst_res(TBROK, "recvmsg or recvmmsg");

	ensure(tn, (size_t) received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, server_data[twice] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	return 0;
}

/*
 * recvmsg(2):
 *
 *	"Datagram sockets in various domains (e.g., the UNIX and Internet
 *	 domains) permit zero-length datagrams. When such a datagram is
 *	 received, the return value is 0."
 */

STATIC int con_empty_datagram_received_as_empty_datagram(con_t *conp, int tn)
{
	iovec_t client_iov;
	iovec_init(&client_iov, "", 0);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[1];		/* client sends nothing */
	server_data[0] = GUARD_CHAR;	/* set guard */
	iovec_t server_iov;
	iovec_init(&server_iov, server_data, 1);

	sockaddr_in_t client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	msghdr_t server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	recvmsg_call_t rmc;
	recvmsg_call_init(&rmc, &server_recv, 0);

	con_add_send_recv_calls(conp, &smc, &rmc, true);

	ssize_t received = rmc.rmc_value;
	if (received < 0)
		tst_res(TBROK, "recvmsg or recvmmsg");

	ensure(tn, received == 0);
	ensure(tn, server_data[0] == GUARD_CHAR);
	return TEST_NEEDS_SLEEP_IN_SENDER;
}

/*}
 *{	The test functions above have variations based on their arguments.
 *
 *	The variations are wrapped with specific argument values into objects
 *	that derive from call_t, the wrapped objects don't change so they
 *	can be used to repeat a test multiple times for a multi-message call.
 *
 */

call_empty_datagram_received_as_empty_datagram_t edraed_0;
call_receive_iovec_boundary_checks_t ribc_0, ribc_1, ribc_2, ribc_3,
				     ribc_4, ribc_5, ribc_6, ribc_7;
call_receive_iovec_boundary_checks_peeking_t ribcp_0, ribcp_1, ribcp_2, ribcp_3,
					     ribcp_4, ribcp_5, ribcp_6, ribcp_7;
call_receive_file_descriptor_t rfd_0, rfd_1, rfd_2, rfd_3,
			       rfd_4, rfd_5, rfd_6, rfd_7,
			       rfd_8, rfd_9, rfd_10, rfd_11,
			       rfd_12, rfd_13, rfd_14, rfd_15;
call_message_too_long_to_fit_might_discard_bytes_t mtltfmdb_1;

#ifdef RECVMMSG_USE_SCTP
call_message_too_long_to_fit_might_discard_bytes_t mtltfmdb_0;
call_receive_waits_for_message_t rwfm_0;
call_receiver_doesnt_wait_for_messages_t rdwfm_0;
call_receive_returns_what_is_available_t rwis_0;
#endif

call_t *call_tests[] = {
	(call_t *) &edraed_0,
	(call_t *) &mtltfmdb_1,
	(call_t *) &ribc_0, (call_t *) &ribc_1,
	(call_t *) &ribc_2, (call_t *) &ribc_3,
	(call_t *) &ribc_4, (call_t *) &ribc_5,
	(call_t *) &ribc_6, (call_t *) &ribc_7,
	(call_t *) &ribcp_0, (call_t *) &ribcp_1,
	(call_t *) &ribcp_2, (call_t *) &ribcp_3,
	(call_t *) &ribcp_4, (call_t *) &ribcp_5,
	(call_t *) &ribcp_6, (call_t *) &ribcp_7,
	(call_t *) &rfd_0, (call_t *) &rfd_1,
	(call_t *) &rfd_2, (call_t *) &rfd_3,
	(call_t *) &rfd_4, (call_t *) &rfd_5,
	(call_t *) &rfd_6, (call_t *) &rfd_7,
	(call_t *) &rfd_8, (call_t *) &rfd_9,
	(call_t *) &rfd_10, (call_t *) &rfd_11,
	(call_t *) &rfd_12, (call_t *) &rfd_13,
	(call_t *) &rfd_14, (call_t *) &rfd_15,
#ifdef RECVMMSG_USE_SCTP
	(call_t *) &mtltfmdb_0,
	(call_t *) &rwfm_0,
	(call_t *) &rdwfm_0,
	(call_t *) &rwis_0,
#endif
};

STATIC void tests_setup(void)
{
	size_t iov_max = iovec_max();

	call_empty_datagram_received_as_empty_datagram_init(&edraed_0);

#ifdef RECVMMSG_USE_SCTP
	call_receive_waits_for_message_init(&rwfm_0);
	call_receiver_doesnt_wait_for_messages_init(&rdwfm_0);
	call_receive_returns_what_is_available_init(&rwis_0);
	call_message_too_long_to_fit_might_discard_bytes_init(&mtltfmdb_0,
							      SOCK_SEQPACKET);
#endif

	call_message_too_long_to_fit_might_discard_bytes_init(&mtltfmdb_1,
							      SOCK_DGRAM);

	call_receive_iovec_boundary_checks_init(&ribc_0, 1);
	call_receive_iovec_boundary_checks_init(&ribc_1, 2);
	call_receive_iovec_boundary_checks_init(&ribc_2, 3);
	call_receive_iovec_boundary_checks_init(&ribc_3, iov_max - 2);
	call_receive_iovec_boundary_checks_init(&ribc_4, iov_max - 1);
	call_receive_iovec_boundary_checks_init(&ribc_5, iov_max);
	call_receive_iovec_boundary_checks_init(&ribc_6, iov_max + 1);
	call_receive_iovec_boundary_checks_init(&ribc_7, iov_max + 2);

	call_receive_iovec_boundary_checks_peeking_init(&ribcp_0, 1);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_1, 2);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_2, 3);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_3, iov_max - 2);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_4, iov_max - 1);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_5, iov_max);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_6, iov_max + 1);
	call_receive_iovec_boundary_checks_peeking_init(&ribcp_7, iov_max + 2);

	bool some_data = true;
	bool no_data = false;

	call_receive_file_descriptor_init(&rfd_0, 0, some_data, 0);
	call_receive_file_descriptor_init(&rfd_1, 0, no_data, 0);
	call_receive_file_descriptor_init(&rfd_2, MSG_CMSG_CLOEXEC,
					  some_data, 0);
	call_receive_file_descriptor_init(&rfd_3, MSG_CMSG_CLOEXEC, no_data, 0);

	/*
	 * ctlmsgfd_t (with 64 bit size_t) gets an extra int of padding
	 * missing bytes then has to be sizeof(size_t) not sizeof(int),
	 * otherwise there is still space for a sizeof(int) sized fd
	 */

	call_receive_file_descriptor_init(&rfd_4, 0, some_data,
					  -sizeof(size_t));
	call_receive_file_descriptor_init(&rfd_5, 0, no_data,
					  -sizeof(size_t));
	call_receive_file_descriptor_init(&rfd_6, MSG_CMSG_CLOEXEC, some_data,
					  -sizeof(size_t));
	call_receive_file_descriptor_init(&rfd_7, MSG_CMSG_CLOEXEC, no_data,
					  -sizeof(size_t));

	/* missing actual fields in the underlying cmsghdr_t */

	call_receive_file_descriptor_init(&rfd_8, 0, some_data,
					  -(ssize_t) (2 * sizeof(size_t)));
	call_receive_file_descriptor_init(&rfd_9, 0, no_data,
					  -(ssize_t) (2 * sizeof(size_t)));
	call_receive_file_descriptor_init(&rfd_10, MSG_CMSG_CLOEXEC, some_data,
					  -(ssize_t) (2 * sizeof(size_t)));
	call_receive_file_descriptor_init(&rfd_11, MSG_CMSG_CLOEXEC, no_data,
					  -(ssize_t) (2 * sizeof(size_t)));

	/* just one byte in the underlying cmsghdr_t */

	call_receive_file_descriptor_init(&rfd_12, 0, some_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
	call_receive_file_descriptor_init(&rfd_13, 0, no_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
	call_receive_file_descriptor_init(&rfd_14, MSG_CMSG_CLOEXEC, some_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
	call_receive_file_descriptor_init(&rfd_15, MSG_CMSG_CLOEXEC, no_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
}

STATIC void tests_run(void)
{
	tests_run_individually();
	tests_run_all_socketpair();
#ifdef RECVMMSG_USE_SCTP
	tests_run_all_sock_sepacket();
	tests_run_all_sock_sepacket_with_n_repeat();
#endif
	tests_run_all_ocloexec_flag();
}

/*}
 *{	Test selection and combination into multi-message tests.
 */

STATIC int tests_select(callconinit_t cci, call_t *tests_src[],
			call_t *tests_dest[], size_t n, int error)
{
	int d = 0;
	for (size_t s = 0; s < n; ++s) {
		call_t *callp = tests_src[s];
		if (callp->call_con_init == cci && callp->call_errno == error) {
			tests_dest[d] = callp;
			++d;
		}
	}
	return d;
}

enum { N_TESTS = sizeof(call_tests) / sizeof(call_tests[1]) };

STATIC void tests_run_individually(void)
{
	if (test_verbose > 1)
		tst_res(TINFO, "Running %d individual sendmsg(2) tests:\n",
			N_TESTS);
	int i;
	for (i = 0; i < N_TESTS; ++i) {
		call_t *callp = call_tests[i];
		con_t con;
		callp->call_con_init(callp, &con);
		callp->call_errno = call_go(callp, &con);
		con_deinit(&con);
	}
}

STATIC void tests_run_all_socketpair(void)
{
	con_t con;
	int error;
	call_t *callp;
	size_t max_tests = iovec_max();
	assert(max_tests > N_TESTS);
	call_t *call_tests_selected[max_tests + 1];
	sendmsg_call_t *smc_vec[max_tests];
	recvmsg_call_t *rmc_vec[max_tests];

	/*
	 * Select all the socketpair() based test cases a test array so
	 * they can be used together in single recvmmsg() multi-message test
	 * that uses the same connection.
	 */

	int n_selected = tests_select(call_do_con_init_socketpair,
				      call_tests, call_tests_selected,
				      N_TESTS, 0);

	/*
	 * Call them individually sharing the connection one after the 
	 * other to ensure there are no dependencies, for example left-
	 * over data leakage left in the socket between individual tests
	 * that might break the tests
	 */

	if (test_verbose > 1)
		tst_res(TINFO, "Running %d non-error socketpair() "
			"tests sharing a connection:\n", (int) n_selected);
	con_init_socketpair(&con);
	int i;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		error = call_go(callp, &con);
		assert(!error);
	}
	con_deinit(&con);

	/*
	 * We have n_selected tests that can work over the same type of
	 * connection and don't have any failures. Use them for mult-receive
	 * recvmmsg(2) testing.
	 *
	 * Calling the first test causes it to call in the middle of it
	 * (unknown to it) the next one in a nested recursive call, the
	 * same occurs for each one of them, when there are no more to
	 * recurse all the sendmsg_call_t and recvmsg_call_t have been
	 * gathered and they can be used to craft the single recvmmsg(2)
	 * call which is tested both with n_selected sendmsg(2) calls or
	 * a single call to sendmmsg(2).
	 *
	 * The recursion occurs, unknown by the tests, in their call to:
	 *	con_add_send_recv_calls()
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		con_init_socketpair(&con);
		if (test_verbose > 1)
			tst_res(TINFO, "Running %d non-error socketpair() tests"
				" with a single recvmmsg(2) call and %s:\n",
				n_selected,
				sendmulti ? "a single sendmmsg(2) call"
					  : "multiple sendmsg(2) calls");
		con_make_vectored(&con, sendmulti > 0, n_selected,
				  call_tests_selected, smc_vec, rmc_vec);
		callp = call_tests_selected[0];
		error = call_go(callp, &con);
		assert(!error);					
		con_deinit(&con);
	}

	/*
	 * Run the recvmmsg(2) test from above with sleeps in the
	 * sender and MSG_WAITFORONE in the receiver, the receiver
	 * keeps on on re-trying until they are all received.
	 */

	if (test_verbose > 1)
		tst_res(TINFO, "Running %d non-error socketpair() tests with as"
			" many MSG_WAITFORONE recvmmsg(2) calls as needed and"
			" individual sleeping sendmsg(2) calls:\n", n_selected);
	con_init_socketpair(&con);
	con_make_vectored(&con, false, n_selected,
			  call_tests_selected, smc_vec, rmc_vec);
	con_make_waitforone_tests(&con);
	callp = call_tests_selected[0];
	error = call_go(callp, &con);
	assert(!error);					
	con_deinit(&con);

	/*
	 * Run the recvmmsg(2) test from above with sleeps in the
	 * sender and timeouts in the receiver, the receiver keeps
	 * on on re-trying until they are all received.
	 */

	if (test_verbose > 1)
		tst_res(TINFO, "Running %d non-error socketpair() tests with as"
			" many recvmmsg(2) calls with a timeout as needed and"
			" individual sleeping sendmsg(2) calls:\n", n_selected);
	con_init_socketpair(&con);
	con_make_vectored(&con, false, n_selected,
			  call_tests_selected, smc_vec, rmc_vec);
	con_make_timeout_tests(&con);
	callp = call_tests_selected[0];
	error = call_go(callp, &con);
	assert(!error);					
	con_deinit(&con);
}

STATIC void tests_run_all_ocloexec_flag(void)
{
	con_t con;
	int error;
	size_t max_tests = iovec_max();
	assert(max_tests > N_TESTS);
	call_t *call_tests_selected[max_tests + 1];
	sendmsg_call_t *smc_vec[max_tests];
	recvmsg_call_t *rmc_vec[max_tests];

	/*
	 * Get the socketpair tests that returned TEST_CLOEXEC_FLAG
	 * and run them sequentially to ensure there are no issues with
	 * the connection sharing.
	 */

	int n_selected = tests_select(call_do_con_init_socketpair, call_tests,
				      call_tests_selected,
				      N_TESTS, TEST_CLOEXEC_FLAG);
	if (test_verbose > 1)
		tst_res(TINFO, "Running %d TEST_CLOEXEC_FLAG socketpair() "
			"tests sharing a connection:\n", n_selected);
	con_init_socketpair(&con);
	int i;
	call_t *callp;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int error = call_go(callp, &con);
		assert(error == TEST_CLOEXEC_FLAG);
	}
	con_deinit(&con);

	/*
	 * Now run them with recvmmsg(2)
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			tst_res(TINFO, "Running %d TEST_CLOEXEC_FLAG "
				"socketpair() tests with a single recvmmsg(2) "
				"call and %s:\n", n_selected,
				sendmulti ? "a single sendmmsg(2) call"
					  : "multiple sendmsg(2) calls");
		con_init_socketpair(&con);
		con_make_vectored(&con, sendmulti > 0, n_selected,
				  call_tests_selected, smc_vec, rmc_vec);
		callp = call_tests_selected[0];
		error = call_go(callp, &con);
		assert(error == TEST_CLOEXEC_FLAG);
		con_deinit(&con);
	}
}

#ifdef RECVMMSG_USE_SCTP /*{*/

STATIC void tests_run_all_sock_sepacket(void)
{
	con_t con;
	int error;
	size_t max_tests = iovec_max();
	assert(max_tests > N_TESTS);
	call_t *call_tests_selected[max_tests + 1];
	sendmsg_call_t *smc_vec[max_tests];
	recvmsg_call_t *rmc_vec[max_tests];

	/*
	 * Select the SOCK_SEQPACKET tests, run them sequentially sharing
	 * the connection to ensure there are no issues sharing it.
	 */

	int n_selected = tests_select(call_do_con_init_seqpacket, call_tests,
				      call_tests_selected, N_TESTS, 0);
	if (test_verbose > 1)
		tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
			"tests sharing a connection:\n", n_selected);
	con_init_seqpacket(&con);
	int i;
	call_t *callp;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		error = call_go(callp, &con);
		assert(!error);
	}
	con_deinit(&con);

	/*
	 * Run the SOCK_SEQPACKET tests with multiple sendmsg(2) calls
	 * (or a single sendmmsg(2) call) and recvmmsg(2).
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
				"tests with a single recvmmsg(2) call "
				"and %s:\n", n_selected,
				sendmulti ? "a single sendmmsg(2) call"
					  : "multiple sendmsg(2) calls");
		con_init_seqpacket(&con);
		con_make_vectored(&con, sendmulti > 0, n_selected,
				  call_tests_selected, smc_vec, rmc_vec);
		callp = call_tests_selected[0];
		error = call_go(callp, &con);
		assert(!error);					
		con_deinit(&con);
	}
}

enum { N_REPEAT = 4 };

STATIC void tests_run_all_sock_sepacket_with_n_repeat(void)
{
	con_t con;
	int error;
	size_t iov_max = iovec_max();
	size_t max_tests = iov_max;
	assert(max_tests > N_TESTS);
	call_t *call_tests_selected[max_tests + 1];
	sendmsg_call_t *smc_vec[max_tests];
	recvmsg_call_t *rmc_vec[max_tests];

	/*
	 * Get the SOCK_SEQPACKET tests again, repeat them N_REPEAT times
	 * and run them sequentially to ensure there are no issues with the
	 * connection sharing.
	 */

	int n_selected = tests_select(call_do_con_init_seqpacket, call_tests,
				      call_tests_selected, N_TESTS, 0);
	assert(N_REPEAT * n_selected < N_TESTS);
	int i;
	call_t *callp;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int skip, r;
		for (skip = n_selected, r = 0; r < N_REPEAT;
		     ++r, skip += n_selected)
			call_tests_selected[i + skip] = callp;
	}

	n_selected *= N_REPEAT;
	if (test_verbose > 1)
		tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
			"tests sharing a connection (N_REPEAT = %d):\n",
			n_selected, N_REPEAT);
	con_init_seqpacket(&con);
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		error = call_go(callp, &con);
		assert(!error);
	}
	con_deinit(&con);

	/*
	 * Now run them with recvmmsg(2)
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
				"tests with a single recvmmsg(2) call "
				"(N_REPEAT = %d) and %s:\n",
				n_selected, N_REPEAT,
				sendmulti ? "a single sendmmsg(2) call"
					  : "multiple sendmsg(2) calls");
		con_init_seqpacket(&con);
		con_make_vectored(&con, sendmulti > 0, n_selected,
				  call_tests_selected, smc_vec, rmc_vec);
		callp = call_tests_selected[0];
		error = call_go(callp, &con);
		assert(!error);					
		con_deinit(&con);
	}

	/*
	 * Repeat the first test iov_max with recvmmsg(2)
	 */

	n_selected = iov_max;
	callp = call_tests_selected[0];
	for (i = 1; i < n_selected; ++i)
		call_tests_selected[i] = callp;

	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
				"tests with a single recvmmsg(2) call "
				"and %s:\n", n_selected,
				sendmulti ? "a single sendmmsg(2) call"
					  : "multiple sendmsg(2) calls");
		con_init_seqpacket(&con);
		con_make_vectored(&con, sendmulti > 0, n_selected,
				  call_tests_selected, smc_vec, rmc_vec);
		callp = call_tests_selected[0];
		error = call_go(callp, &con);
		assert(!error);					
		con_deinit(&con);
	}
}

#endif /*}*/

/*
 *	The LTP coding convention is for main() not to be provided by
 *	the test programs, for easier debugging you might want to use
 *	this main() function (for easier testing instead of having to go
 *	find the LTP library code that the LTP provided main() function
 *	uses).
 *
 *	If you want to use this main() function add this #define:
 *		#define TST_NO_DEFAULT_MAIN
 *	Prior to this #include:
 *		#include "tst_test.h"
 *	at the start of this file.
 *
 *	Another reason you might want to do this is to pass one or more:
 *		-v
 *	to the test program to increse its verbosity level, alternatively
 *	you can initialize test_verbose to a non-zero value and recompile.
 */

#ifdef TST_NO_DEFAULT_MAIN

STATIC void tst_parse_opt(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	tst_parse_opt(argc, argv);
	tests_setup();
	tests_run();
	exit(ensure_failures > 0 ? 1 : 0);
}

STATIC void tst_parse_opt(int argc, char *argv[])
{
	prog_name = argv[0];
	char *lasts = strrchr(prog_name, '/');
	if (lasts)
		prog_name = lasts + 1;
	TCID = prog_name;
	while (--argc >= 1) {
		char *arg = *++argv;
		if (strcmp(arg, "-v") == 0)
			++test_verbose;
		else {
			fprintf(stderr, "usage: %s [-v]\n", prog_name);
			exit(1);
		}
	}
}

#else

static void tests_cleanup(void);
static void tests_cleanup(void) {}

static struct tst_test test = {
	.setup = tests_setup,
	.cleanup = tests_cleanup,
	.test_all = tests_run,
};

#endif

/*}
 *{	Utility functions and mechanism for test combinations and for
 *	turning individual sendmsg(2) / recvmsg(2) based tests into
 *	multi-message sendmmsg(2) / recvmmsg(2) calls are in this code.
 *
 *	Test functions that use a single sendmsg() and a single recvmsg() use:
 *		con_add_send_recv_calls()
 *
 *	To either run the test with an individual sendmsg() and recvmsg(),
 *	or to have those combined with the send and receive of unrelated
 *	tests into a multi-message test.
 *
 *	The combination happens in:
 *		con_do_multi_send_recv()
 */

STATIC void con_init_base(con_t *conp, int client, int server,
			  int type, sockaddr_in_t *sinp)
{
	conp->con_type = type;
	conp->con_client = client;
	conp->con_server = server;
	if (!sinp)
		memset(&conp->con_server_sin, 0 , sizeof(conp->con_server_sin));
	else
		conp->con_server_sin = *sinp;
}

STATIC void con_init(con_t *conp, int client, int server,
		     int type, sockaddr_in_t *sinp)
{
	con_init_base(conp, client, server, type, sinp);
	conp->con_call_sendmmsg = false;
	conp->con_sender_sleeps = false;	/* determined later */
	conp->con_waitforone = false;
	conp->con_timeout = false;
	conp->con_call_end = 0;
	conp->con_call_n = 0;
	conp->con_call_vec = NULL;
	conp->con_smc_vec = NULL;
	conp->con_rmc_vec = NULL;
}

STATIC void con_make_vectored(con_t *conp, bool sendmulti,
			      size_t n, call_t **call_vec,
			      sendmsg_call_t **smc_vec,
			      recvmsg_call_t **rmc_vec)
{
	assert(n >= 1 && call_vec && smc_vec && rmc_vec);
	assert(conp->con_client >= 0 &&
	       conp->con_server >= 0 &&
	       !conp->con_call_sendmmsg &&
	       !conp->con_call_end &&
	       !conp->con_call_n &&
	       !conp->con_call_vec &&
	       !conp->con_smc_vec &&
	       !conp->con_rmc_vec);
	conp->con_call_sendmmsg = sendmulti;
	conp->con_call_end = 0;
	conp->con_call_n = n;
	conp->con_call_vec = call_vec;
	conp->con_smc_vec = smc_vec;
	conp->con_rmc_vec = rmc_vec;
}

STATIC void con_make_waitforone_tests(con_t *conp)
{
	assert(conp->con_call_vec);
	assert(!conp->con_call_sendmmsg);
	conp->con_timeout = true;
}

STATIC void con_make_timeout_tests(con_t *conp)
{
	assert(conp->con_call_vec);
	assert(!conp->con_call_sendmmsg);
	conp->con_waitforone = true;
}

STATIC void con_add_send_recv_calls_vec(con_t *conp, sendmsg_call_t *smcp,
					recvmsg_call_t *rmcp,
					bool sender_sleeps)
{
	assert(conp->con_call_end < conp->con_call_n);
	size_t i = conp->con_call_end;

	/*
	 * All senders must either: need to sleep or must not sleep.
	 */

	if (i == 0)
		conp->con_sender_sleeps = sender_sleeps;
	else
		assert(conp->con_sender_sleeps == sender_sleeps);
	conp->con_smc_vec[i] = smcp;
	conp->con_rmc_vec[i] = rmcp;

	++i;
	conp->con_call_end = i;
	if (i == conp->con_call_n)
		con_do_multi_send_recv(conp);
	else {
		subtest_nest();
		call_t *callp = conp->con_call_vec[i];
		int error = call_go(callp, conp);
		assert(!errno_value_is_error(error));
		subtest_unnest();
	}
}

STATIC void *sendmmsg_call(sendmmsg_call_t *smmcp)
{
	errno = 0;

	int sockfd = smmcp->smmc_sockfd;
	int vlen = smmcp->smmc_vlen;
	mmsghdr_t *smm = smmcp->smmc_smm;;
	int flags = smmcp->smmc_flags;
	bool call_sendmmsg = smmcp->smmc_call_sendmmsg;
	bool sender_sleeps = smmcp->smmc_sender_sleeps;

	int nsent;
	if (call_sendmmsg)
		nsent = sendmmsg(sockfd, smm, vlen, flags);
	else {
		for (nsent = 0; nsent < vlen; ++nsent) {
			if (sender_sleeps)
				sleep_ms(SENDMSG_SLEEP_MS);
			int value = sendmsg(sockfd, &smm[nsent].msg_hdr, flags);
			assert(value >= 0);
			smm[nsent].msg_len = value;
		}
	}

	smmcp->smmc_value = nsent;
	smmcp->smmc_errno = errno;
	return NULL;
}

STATIC void con_sendmmsg_recvmmsg(con_t *conp, size_t n,
				  int sflags, mmsghdr_t *smm,
				  int rflags, mmsghdr_t *rmm)
{
	bool waitforone = conp->con_waitforone;
	bool timeout = conp->con_timeout;
	assert(!waitforone || !timeout);

	sendmmsg_call_t smmc;
	sendmmsg_init(&smmc, conp->con_client, (unsigned) n,
		      smm, sflags, conp->con_call_sendmmsg,
		      waitforone || timeout || conp->con_sender_sleeps);

	thread_call(&smmc.smmc_thread_call, (thread_start_t) sendmmsg_call);

	int nrecv;
	if (!waitforone && !timeout) {
		nrecv = recvmmsg(conp->con_server, rmm, n, rflags, NULL);
		assert((size_t) nrecv == n);
	} else {
		timespec_t ts, *tsp = NULL;
		if (timeout) {
			ts.tv_sec = 0;
			ts.tv_nsec = 100 * 1000 * 1000;
			tsp = &ts;
		}
		if (waitforone)
			rflags |= MSG_WAITFORONE;
		int nleft = n;
		do {
			nrecv = recvmmsg(conp->con_server, rmm,
					 nleft, rflags, tsp);
			if (waitforone)
				assert(nrecv > 0);
			if (timeout && nrecv == -1) {
				nrecv = 0;
				assert(errno == ETIMEDOUT ||
				       errno == EAGAIN ||
				       errno == EWOULDBLOCK);
			}
			nleft -= nrecv;
			rmm += nrecv;
		} while (nleft > 0);
	}

	thread_join(&smmc.smmc_thread_call);

	int nsent = smmc.smmc_value;
	assert((size_t) nsent == n);
}

STATIC void con_do_multi_send_recv(con_t *conp)
{
	assert(conp->con_call_n >= 1 && conp->con_call_n <= iovec_max());
	size_t n = conp->con_call_n;
	mmsghdr_t smm[n];
	mmsghdr_t rmm[n];

	/*
	 * Gather the arguments from all the sendmsg_call_t and the
	 * recvmsg_call_t into the smm[] and rmm[] vectors for the
	 * multi-message send and receive syscalls.
	 *
	 * All the send and recv flags must be the same, there is
	 * a single argument for the flags in sendmmsg(2) and in
	 * recvmmsg(2), there is not an argument per message.
	 */

	int sflags = conp->con_smc_vec[0]->smc_flags;
	int rflags = conp->con_rmc_vec[0]->rmc_flags;
	size_t i;
	for (i = 0; i < n; i++) {
		assert(conp->con_smc_vec[i]->smc_flags == sflags);
		assert(conp->con_rmc_vec[i]->rmc_flags == rflags);
		mmsghdr_init(&smm[i], conp->con_smc_vec[i]->smc_msg);
		mmsghdr_init(&rmm[i], conp->con_rmc_vec[i]->rmc_msg);
	}

	con_sendmmsg_recvmmsg(conp, n, sflags, smm, rflags, rmm);

	/*
	 * Scatter the results from smm[] and rmm[].
	 */

	for (i = 0; i < n; i++) {
		conp->con_smc_vec[i]->smc_value = smm[i].msg_len;
		conp->con_smc_vec[i]->smc_msg->msg_flags =
						  smm[i].msg_hdr.msg_flags;
		conp->con_rmc_vec[i]->rmc_value = rmm[i].msg_len;
		conp->con_rmc_vec[i]->rmc_msg->msg_flags =
						  rmm[i].msg_hdr.msg_flags;
	}
}

STATIC void con_init_socketpair(con_t *conp)
{
	int pair[2];
	SAFE_SOCKETPAIR(AF_UNIX, SOCK_SEQPACKET, 0, pair);
	con_init(conp, pair[0], pair[1], SOCK_SEQPACKET, NULL);
}

STATIC void con_init_seqpacket(con_t *conp)
{
	con_init_with_type(conp, SOCK_SEQPACKET);
}

STATIC void con_init_dgram(con_t *conp)
{
	con_init_with_type(conp, SOCK_DGRAM);
}

STATIC void con_init_with_type(con_t *conp, int type)
{
#ifndef RECVMMSG_USE_SCTP
	assert(type != SOCK_SEQPACKET);
#endif
	int protocol = (type == SOCK_SEQPACKET) ? IPPROTO_SCTP : 0;
	int server = SAFE_SOCKET(PF_INET, type, protocol);

	sockaddr_in_t sin;
	sockaddr_in_init(&sin, AF_INET, TEST_PORT, TEST_IP);
	SAFE_BIND(server, (sockaddr_t *) &sin, sizeof(sin));

	int client = SAFE_SOCKET(PF_INET, type, protocol);

	if (type == SOCK_SEQPACKET)
		SAFE_LISTEN(server, 1);

	con_init(conp, client, server, type, &sin);
}

STATIC void con_deinit(con_t *conp)
{
	close(conp->con_client);
	close(conp->con_server);
}

STATIC void con_add_send_recv_calls(con_t *conp, sendmsg_call_t *smcp,
				    recvmsg_call_t *rmcp, bool sender_sleeps)
{
	if (conp->con_call_n > 0) {
		con_add_send_recv_calls_vec(conp, smcp, rmcp, sender_sleeps);
		return;
	}

	thread_call(&smcp->smc_thread_call, (thread_start_t)
		    (sender_sleeps ? sendmsg_call_after_sleep : sendmsg_call));
	timeval_t pretv, postv;
	if (sender_sleeps)
		SAFE_GETTIMEOFDAY(&pretv, NULL);
	ssize_t val = recvmsg(conp->con_server, rmcp->rmc_msg, rmcp->rmc_flags);
	if (val < 0)
		rmcp->rmc_errno = errno;

	if (sender_sleeps) {
		/*
		 * If the sender is supposed to sleep prior to sending then
		 * the receiver is supposed to sleep too because its receive
		 * should block for the sender, testing that sender blocked
		 * for at least half as long as the sender sleeps is good
		 * enough to ensure that scheduling noise doesn't affect test.
		 */
		SAFE_GETTIMEOFDAY(&postv, NULL);
		unsigned long long pre = pretv.tv_sec * 1000;
		pre += pretv.tv_usec * 1000;
		unsigned long long pos = postv.tv_sec * 1000;
		pos += postv.tv_usec * 1000;
		assert((pos - pre) > SENDMSG_SLEEP_MS / 2);
	}

	rmcp->rmc_value = val;
	thread_join(&smcp->smc_thread_call);
}

/*}
 *{	Initialization of test functions into wrapped tests into classes
 *	that derive from call_t. For each test function there is an init
 *	function and a test running function. All the test running funcitons
 *	have the same signature, so the calling mechanism doesn't have to
 *	know about their arguments which are wrapped into the classes that
 *	inherit from call_t.
 *
 *	Tests are bundled into their own call_t types so they can be mixed
 *	into multi-message sends and receives (sendmmsg(2) and recvmmsg(2)
 */

STATIC void call_receive_iovec_boundary_checks_init(
		call_receive_iovec_boundary_checks_t *callp, size_t niov)
{
	call_base_init(&callp->call_base, call_do_con_init_socketpair,
		       (calltest_t) call_receive_iovec_boundary_checks);
	callp->call_niov = niov;
}

STATIC int call_receive_iovec_boundary_checks(
		call_receive_iovec_boundary_checks_t *callp,
		con_t *conp, int tn)
{
	int r = con_receive_iovec_boundary_checks(conp, tn, callp->call_niov);
	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_iovec_boundary_checks(niov = %d)"
			" // iov_max = %d",
			(int) callp->call_niov, (int) iovec_max());
	return r;
}

STATIC void call_receive_iovec_boundary_checks_peeking_init(
		call_receive_iovec_boundary_checks_peeking_t *callp,
		size_t niov)
{
	call_base_init(&callp->call_base, call_do_con_init_socketpair,
		       (calltest_t) call_receive_iovec_boundary_checks_peeking);
	callp->call_niov = niov;
}

STATIC int call_receive_iovec_boundary_checks_peeking(
		call_receive_iovec_boundary_checks_peeking_t *callp,
		con_t *conp, int tn)
{
	int r = con_receive_iovec_boundary_checks_peeking(conp, tn,
							  callp->call_niov);
	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_iovec_boundary_checks_peeking(niov"
			" = %d) // iov_max = %d",
			(int) callp->call_niov, (int) iovec_max());
	return r;
}

STATIC void call_receive_file_descriptor_init(
		call_receive_file_descriptor_t *callp,
		int cloexec_flag, bool some_data, ssize_t controllen_delta)
{
	call_base_init(&callp->call_base, call_do_con_init_socketpair,
		       (calltest_t) call_receive_file_descriptor);
	callp->call_cloexec_flag = cloexec_flag;
	callp->call_some_data = some_data;
	callp->call_controllen_delta = controllen_delta;
}

STATIC int call_receive_file_descriptor(
		call_receive_file_descriptor_t *callp, con_t *conp, int tn)
{
	int r = con_receive_file_descriptor(conp, tn, callp->call_cloexec_flag,
					    callp->call_some_data,
					    callp->call_controllen_delta);
	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_file_descriptor("
			"cloexec_flag = %s, some_data = %d, "
			"controllen_delta = %ld)",
			(callp->call_cloexec_flag & MSG_CMSG_CLOEXEC) ?
				"MSG_CMSG_CLOEXEC" : "0",
			callp->call_some_data,
			(long) callp->call_controllen_delta);
	return r;
}

STATIC void call_msgtoolong_do_con_init_from_type(
	call_message_too_long_to_fit_might_discard_bytes_t *callp,
	con_t *conp)
{
	con_init_with_type(conp, callp->call_type);
}

STATIC void call_message_too_long_to_fit_might_discard_bytes_init(
		call_message_too_long_to_fit_might_discard_bytes_t *callp,
		int type)
{
	call_base_init(&callp->call_base,
		       (callconinit_t) call_msgtoolong_do_con_init_from_type,
		       (calltest_t)
		       call_message_too_long_to_fit_might_discard_bytes);
	callp->call_type = type;
}

STATIC int call_message_too_long_to_fit_might_discard_bytes(
		call_message_too_long_to_fit_might_discard_bytes_t *callp,
		con_t *conp, int tn)
{
	int r = con_message_too_long_to_fit_might_discard_bytes(conp, tn,
							callp->call_type);
	if (test_verbose || subtest_show())
		tst_res(TINFO,
			"con_message_too_long_to_fit_might_discard_bytes"
			"(type = %s)", sock_type_to_str(callp->call_type));
	return r;
}

STATIC void call_empty_datagram_received_as_empty_datagram_init(
		call_empty_datagram_received_as_empty_datagram_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_dgram,
		       (calltest_t)
		       call_empty_datagram_received_as_empty_datagram);
}

STATIC int call_empty_datagram_received_as_empty_datagram(
		call_empty_datagram_received_as_empty_datagram_t *callp,
		con_t *conp, int tn)
{
	unused(callp);
	int r = con_empty_datagram_received_as_empty_datagram(conp, tn);
	if (test_verbose || subtest_show())
		tst_res(TINFO,
			"con_empty_datagram_received_as_empty_datagram()");
	return r;
}

#ifdef RECVMMSG_USE_SCTP /*{*/

STATIC void call_receive_waits_for_message_init(
		call_receive_waits_for_message_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_seqpacket,
		       (calltest_t) call_receive_waits_for_message);
}

STATIC int call_receive_waits_for_message(
		call_receive_waits_for_message_t *callp, con_t *conp, int tn)
{
	unused(callp);
	int r = con_receive_waits_for_message(conp, tn);
	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_waits_for_message()");
	return r;
}

STATIC void call_receiver_doesnt_wait_for_messages_init(
		call_receiver_doesnt_wait_for_messages_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_seqpacket,
		       (calltest_t) call_receiver_doesnt_wait_for_messages);
}

STATIC int call_receiver_doesnt_wait_for_messages(
		call_receiver_doesnt_wait_for_messages_t *callp,
		con_t *conp, int tn)
{
	unused(callp);
	int r = con_receiver_doesnt_wait_for_messages(conp, tn);
	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receiver_doesnt_wait_for_messages()");
	return r;
}

STATIC void call_receive_returns_what_is_available_init(
		call_receive_returns_what_is_available_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_seqpacket,
		       (calltest_t) call_receive_returns_what_is_available);
}

STATIC int call_receive_returns_what_is_available(
		call_receive_returns_what_is_available_t *callp,
		con_t *conp, int tn)
{
	unused(callp);
	int r = con_receive_returns_what_is_available(conp, tn);
	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_returns_what_is_available()");
	return r;
}

#endif /*}*/

/*}
 *{	Miscellaneous supporting code is in this section.
 */

STATIC int errno_value_is_error(int e)
{
	return e > 0;  /* negative values are not errors, see non_error_errno */
}

STATIC void sleep_ms(long ms)
{
	assert(ms < 1000);
	timespec_t ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ms * 1000 * 1000;
	nanosleep(&ts, NULL);
}

STATIC int test_number_get(void)
{
	static int test_number_gen;

	/*
	 * Nested subtests share the same test number, these are multi
	 * message tests, i.e. recvmmsg() and sendmmsg(), where each message
	 * is treated as a subtest
	 */

	if (subtest_number() > 1)
		return test_number_gen;
	return ++test_number_gen;
}

/*
 * subtests numbers
 */

STATIC int subtest_level = 1;

STATIC int subtest_number(void)
{
	return subtest_level;
}

STATIC void subtest_nest(void)
{
	++subtest_level;
}

STATIC void subtest_unnest(void)
{
	--subtest_level;
}

STATIC bool subtest_show(void)
{
	/*
	 *	Some tests involve too many subtests, this limits the output.
	 */
	return subtest_level <= 20;
}

/*
 * compare that memory is equal, makes the output of various ensure() nicer
 */

STATIC bool mem_is_equal(void *a, void *b, size_t size)
{
	return !memcmp(a, b, size);
}

STATIC bool mem_is_zero(void *a, size_t size)
{
	unsigned char set = 0;
	unsigned char *p = a;
	unsigned char *endp = p + size;
	while (p < endp) set |= *p++;
	return !set;
}

STATIC void thread_call(thread_call_t *tcp, thread_start_t func)
{
	SAFE_PTHREAD_CREATE(&tcp->tc_thr, NULL, func, tcp);
}

STATIC void thread_join(thread_call_t *tcp)
{
	SAFE_PTHREAD_JOIN(tcp->tc_thr, NULL);
}

STATIC void sendmmsg_init(sendmmsg_call_t *smmcp, int sockfd, unsigned vlen,
			  mmsghdr_t *smm, int flags, bool call_sendmmsg,
			  bool sender_sleeps)
{
	smmcp->smmc_sockfd = sockfd;
	smmcp->smmc_vlen = vlen;
	smmcp->smmc_smm = smm;
	smmcp->smmc_flags = flags;
	smmcp->smmc_call_sendmmsg = call_sendmmsg;
	smmcp->smmc_sender_sleeps = sender_sleeps;
	smmcp->smmc_value = INVALID_SYSCALL_VALUE;
	smmcp->smmc_errno = 0;
}

/*
 * various init and member functions for various types
 */

STATIC void sockaddr_in_init(sockaddr_in_t *sinp, sa_family_t family,
			     in_port_t port, uint32_t ip)
{
	sockaddr_in_init_to_zero(sinp);
	sinp->sin_family = family;
	sinp->sin_port = htons(port);
	sinp->sin_addr.s_addr = htonl(ip);
}

STATIC void sockaddr_in_init_to_zero(sockaddr_in_t *sinp)
{
	memset(sinp, 0, sizeof(*sinp));
}

STATIC char *sock_type_to_str(int type)
{
	switch (type) {
	case SOCK_DCCP:		return "SOCK_DCCP";
	case SOCK_DGRAM:	return "SOCK_DGRAM";
	case SOCK_PACKET:	return "SOCK_PACKET";
	case SOCK_RAW:		return "SOCK_RAW";
	case SOCK_RDM:		return "SOCK_RDM";
	case SOCK_SEQPACKET:	return "SOCK_SEQPACKET";
	case SOCK_STREAM:	return "SOCK_STREAM";
	default:		return "socket_type_unknown!";
	}
}

STATIC size_t iovec_max(void)
{
	long iov_max = SAFE_SYSCONF(_SC_IOV_MAX);
	assert(iov_max > 2);
	return (size_t) iov_max;
}

STATIC void iovec_init(iovec_t *iovecp, void *base, size_t len)
{
	iovecp->iov_base = base;
        iovecp->iov_len = len;
}

STATIC void msghdr_init(msghdr_t *msgp, sockaddr_in_t *sinp,
			iovec_t *iovecp, size_t iovlen, int flags)
{
	msghdr_init_with_control(msgp, sinp, iovecp, iovlen, NULL, 0, flags);
}

STATIC void msghdr_init_with_control(msghdr_t *msgp, sockaddr_in_t *sinp,
				     iovec_t *iovecp, size_t iovlen,
				     void *control, size_t controllen,
				     int flags)
{
	memset(msgp, 0, sizeof(*msgp));
	msgp->msg_name = sinp;
	msgp->msg_namelen = sinp ? sizeof(*sinp) : 0;
	msgp->msg_iov =iovecp;
	msgp->msg_iovlen = iovlen;
	msgp->msg_control = control;
	msgp->msg_controllen = controllen;
	msgp->msg_flags = flags;
}

STATIC void mmsghdr_init(mmsghdr_t *mmsgp, msghdr_t *msgp)
{
	mmsgp->msg_hdr = *msgp;
	mmsgp->msg_len = 0;
}

STATIC void recvmsg_call_init(recvmsg_call_t *rmcp, msghdr_t *msgp, int flags)
{
	rmcp->rmc_value = INVALID_SYSCALL_VALUE;
	rmcp->rmc_errno = 0;
	rmcp->rmc_msg = msgp;
	rmcp->rmc_flags = flags;
}

STATIC void sendmsg_call_init(sendmsg_call_t *smcp, int sockfd,
			      msghdr_t *msgp, int flags)
{
	smcp->smc_value = INVALID_SYSCALL_VALUE;
	smcp->smc_errno = 0;
	smcp->smc_sockfd = sockfd;
	smcp->smc_msg = msgp;
	smcp->smc_flags = flags;
}

/*
 * functions that can be called directly through pthread_create(3)
 */

STATIC void *sendmsg_call(sendmsg_call_t *smcp)
{
	errno = 0;
	smcp->smc_value = sendmsg(smcp->smc_sockfd, smcp->smc_msg,
				  smcp->smc_flags);
	smcp->smc_errno = errno;
	return NULL;
}

STATIC void *sendmsg_call_after_sleep(sendmsg_call_t *smcp)
{
	sleep_ms(SENDMSG_SLEEP_MS);
	return sendmsg_call(smcp);
}

STATIC void ctlmsgfd_init(ctlmsgfd_t *cmfp, int fd)
{
	memset(cmfp, 0, sizeof(*cmfp));
	cmfp->cmf_cmsg.cmsg_len = sizeof(*cmfp);
        cmfp->cmf_cmsg.cmsg_level = SOL_SOCKET;
        cmfp->cmf_cmsg.cmsg_type = SCM_RIGHTS;
	*(int *) CMSG_DATA(&cmfp->cmf_cmsg) = fd;
}

STATIC void ctlmsgfd_init_to_zero(ctlmsgfd_t *cmfp)
{
	memset(cmfp, 0, sizeof(*cmfp));
}

STATIC int ctlmsgfd_get_fd(ctlmsgfd_t *cmfp)
{
	return *(int *) CMSG_DATA(&cmfp->cmf_cmsg);
}

STATIC int call_go(call_t *callp, con_t *conp)
{
	static int max_subtest_level = 1;

	int failures = ensure_failures;
	int tn = test_number_get();
	int error = callp->call_test(callp, conp, tn);
	if (subtest_number() > 1) {
		if (subtest_number() > max_subtest_level)
			max_subtest_level = subtest_number();
	} else {
		bool pass = failures == ensure_failures;
		if (max_subtest_level == 1)
			tst_res(pass ? TPASS : TFAIL, "[single test]\n");
		else
			tst_res(pass ? TPASS : TFAIL,
				"[%d tests combined into multi message test]\n",
				max_subtest_level);
		max_subtest_level = 1;
	}
	return error;
}

STATIC void call_do_con_init_socketpair(call_t *callp, con_t *conp)
{
	unused(callp);
	con_init_socketpair(conp);
}

STATIC void call_do_con_init_seqpacket(call_t *callp, con_t *conp)
{
	unused(callp);
	con_init_seqpacket(conp);
}

STATIC void call_do_con_init_dgram(call_t *callp, con_t *conp)
{
	unused(callp);
	con_init_dgram(conp);
}

STATIC void call_base_init(call_t *callp, callconinit_t coninit, calltest_t test)
{
	callp->call_con_init = coninit;
	callp->call_test = test;
	callp->call_errno = 0;
}

/*}*/
