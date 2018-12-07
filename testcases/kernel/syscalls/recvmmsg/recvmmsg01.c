/*
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
#include <time.h>
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
#include "tst_fuzzy_sync.h"

#ifdef HAVE_NETINET_SCTP_H
#define	RUN_SCTP_TESTS	1
#include <netinet/sctp.h>
#else
#define	RUN_SCTP_TESTS	0
#endif

/*
 *	Contants.
 */

#define GUARD_CHAR		'+'	/* guard character to check overwrites*/
#define INVALID_SYSCALL_VALUE	(-2)	/* syscalls values are all >= -1 */
#define TEST_IP			0x7F000001
#define TEST_PORT		6789
#define SENDMSG_SLEEP_MS	10

/*
 * Must be disjoint with errno values and non-zero, value returned by tests
 * that do multiple recvmsg() calls internally, or that have a non-zero flag,
 * or that require special treatment (sender needs to sleep).  Those tests
 * can not be bundled with other tests into a larger recvmmsg(2) multi-
 * receive message test, some can only be bundled with their own kind.
 */

#define TEST_HAS_MULTIPLE_RECVMSG	(-1)	/* can't be bundled at all */
#define TEST_CLOEXEC_FLAG		(-2)	/* bundle these together */
#define TEST_NEEDS_SLEEP_IN_SENDER	(-3)	/* bundle these together */

struct ctlmsgfd {
	struct cmsghdr	cmf_cmsg;
	int		cmf_fdspace;  /* padding in cmf_cmsg implies fd might */
				      /* not be here, might be in cmf_cmsg! */
};

struct thread_call {
	pthread_t	 tc_thr;
};

/*
 * sendmsg(2) call to be sent individually or bundled up with other such
 * calls with sendmmsg(2) by copying them into an struct mmsghdr array, the
 * sending is done by a pthread_create(3) created thread
 */

struct sendmsg_call {
	struct thread_call smc_thread_call;  /* must be first: inherits from */
	int		   smc_sockfd;
	struct msghdr	  *smc_msg;
	int		   smc_flags;
	ssize_t		   smc_value;
	int		   smc_errno;
};

/*
 * sendmmsg(2) call, will be done as a single sendmmsg(2) call or vlen
 * individual sendmsg(2) calls
 */

struct sendmmsg_call {
	struct thread_call smmc_thread_call; /*  must be first: inherits from */
	int		   smmc_sockfd;
	unsigned	   smmc_vlen;
	struct mmsghdr	  *smmc_smm;
	int		   smmc_flags;
	bool		   smmc_call_sendmmsg;
	bool		   smmc_sender_sleeps;
	int		   smmc_value;
	int		   smmc_errno;
};

/*
 * recvmsg(2) call to be received individually or bundled up with other
 * such calls with recvmmsg(2) by copying them into an struct mmsghdr array
 */

struct recvmsg_call {
	ssize_t		 rmc_value;
	int		 rmc_errno;
	struct msghdr	*rmc_msg;
	int		 rmc_flags;
};

/*
 * client server connection
 *
 * A struct con abstracts a connection between a client and a server, it is used
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
 *   In that case the struct con is used to hide the bundling of the tests from
 *   each other and the tests, unsupectingly, pass the test vector via the
 *   con_call_vec and con_call_n members.  Note that con_call_n == 1 is used
 *   as a degenerate case to test recvmmsg(2) with a single message. Note
 *   when recvmmsg(2) is used the messages can be sent individually or in
 *   a single sendmmsg(2) for testing multi-message sending. Both ways of
 *   sending are done controlled by con_call_sendmmsg == true to indicate
 *   used of sendmmsg(2), otherwise multiple sendmsg(2) are used.
 */

struct call;

struct con {
	int		      con_client;
	int		      con_server;
	int		      con_type;
	struct sockaddr_in    con_server_sin;
	bool		      con_call_sendmmsg;
	bool		      con_sender_sleeps;
	bool		      con_waitforone;
	bool		      con_timeout;
	size_t		      con_call_end;  /* index of last+1 filled */
	size_t		      con_call_n;
	struct call	    **con_call_vec;/* points to array of con_call_n */
					   /* pointers to be used in a */
					   /* recvmmsg(2) multi-message test */
	struct sendmsg_call **con_smc_vec;
	struct recvmsg_call **con_rmc_vec;
};

struct call {
	int	     (*call_test)(struct call *, struct con *, int);
	void	     (*call_coninit)(struct con *);
	int	       call_errno;	/* set on first run */
	size_t	       call_niov;
	bool	       call_iov_max_relative;
	int	       call_cloexec_flag;
	bool	       call_some_data;
	ssize_t	       call_controllen_delta;
};

/*
 *	Prototypes.
 */

static void tests_setup(void);
static int tests_select(void (*coninit)(struct con *),
			struct call *chosen[], int error);

static void tests_run(void);
static void tests_run_individually(void);
static void tests_run_all_socketpair(void);
static void tests_run_all_ocloexec_flag(void);
static void tests_run_all_sock_sepacket(void);
static void tests_run_all_sock_sepacket_with_n_repeat(void);

static void sleep_ms(long ms);

static void thread_call(struct thread_call *tcp, void *(*func)(void *));
static void thread_join(struct thread_call *tcp);

static int test_number_get(void);

static int subtest_number(void);
static void subtest_nest(void);
static void subtest_unnest(void);
static bool subtest_show(void);

static bool mem_is_equal(void *a, void *b, size_t size);
static bool mem_is_zero(void *a, size_t size);

static void sockaddr_in_init(struct sockaddr_in *sinp, sa_family_t family,
			     in_port_t port, uint32_t ip);
static void sockaddr_in_init_to_zero(struct sockaddr_in *sinp);

static int errno_value_is_error(int e);

static size_t iovec_max(void);
static void iovec_init(struct iovec *iovecp, void *base, size_t len);

static void msghdr_init(struct msghdr *msgp, struct sockaddr_in *sinp,
			struct iovec *iovecp, size_t iovlen, int flags);
static void msghdr_init_with_control(struct msghdr *msgp,
				     struct sockaddr_in *sinp,
				     struct iovec *iovecp, size_t iovlen,
				     void *control, size_t controllen,
				     int flags);

static void mmsghdr_init(struct mmsghdr *mmsgp, struct msghdr *msgp);

static void recvmsg_call_init(struct recvmsg_call *rmcp,
			      struct msghdr *msgp, int flags);

static void sendmsg_call_init(struct sendmsg_call *smcp, int sockfd,
			      struct msghdr *msgp, int flags);
static void *sendmsg_call(struct sendmsg_call *smcp);
static void *sendmsg_call_after_sleep(struct sendmsg_call *smcp);

static void sendmmsg_init(struct sendmmsg_call *smmcp, int sockfd,
			  unsigned vlen, struct mmsghdr *smm, int flags,
			  bool call_sendmmsg, bool sender_sleeps);

static void ctlmsgfd_init(struct ctlmsgfd *cmfp, int fd);
static void ctlmsgfd_init_to_zero(struct ctlmsgfd *cmfp);
static int ctlmsgfd_get_fd(struct ctlmsgfd *cmfp);

static void con_init(struct con *conp, int client, int server,
		     int type, struct sockaddr_in *sinp);
static void con_init_base(struct con *conp, int client, int server,
			  int type, struct sockaddr_in *sinp);
static void con_make_vectored(struct con *conp, bool sendmulti,
			      size_t n, struct call **call_vec,
			      struct sendmsg_call **smc_vec,
			      struct recvmsg_call **rmc_vec);
static void con_make_waitforone_tests(struct con *conp);
static void con_make_timeout_tests(struct con *conp);
static void con_init_socketpair(struct con *conp);
static void con_init_seqpacket(struct con *conp);
static void con_init_dgram(struct con *conp);
static void con_deinit(struct con *conp);
static void con_sendmmsg_recvmmsg(struct con *conp, size_t n,
				  int sflags, struct mmsghdr *smm,
				  int rflags, struct mmsghdr *rmm);
static void con_do_multi_send_recv(struct con *conp);
static void con_add_send_recv_calls_vec(struct con *conp,
					struct sendmsg_call *smcp,
					struct recvmsg_call *rmcp,
					bool sender_sleeps);
static void con_add_send_recv_calls(struct con *conp,
				    struct sendmsg_call *smcp,
				    struct recvmsg_call *rmcp,
				    bool sender_sleeps);

static int call_go(struct call *callp, struct con *conp);

static int call_receive_iovec_boundary_checks(struct call *callp,
					      struct con *conp, int tn);
static int call_receive_iovec_boundary_checks_peeking(struct call *callp,
						      struct con *conp, int tn);
static int call_receive_file_descriptor(struct call *callp,
					struct con *conp, int tn);
static int call_message_too_long_to_fit_discards_bytes(struct call *callp,
						       struct con *conp,
						       int tn);
static int call_message_too_long_to_fit_doesnt_discard_bytes(struct call *callp,
							     struct con *conp,
							     int tn);
static int call_receive_waits_for_message(struct call *callp,
					  struct con *conp, int tn);
static int call_receiver_doesnt_wait_for_messages(struct call *callp,
						  struct con *conp, int tn);
static int call_receive_returns_what_is_available(struct call *callp,
						  struct con *conp, int tn);
static int call_empty_datagram_received_as_empty_datagram(struct call *callp,
							  struct con *conp,
							  int tn);

static int con_receive_iovec_boundary_checks(struct con *conp,
		int tn, size_t niov);
static int con_receive_iovec_boundary_checks_peeking(struct con *conp, int tn,
						     size_t niov);
static int con_receive_file_descriptor(struct con *conp, int tn,
				       int cloexec_flag,
				       bool some_data,
				       ssize_t controllen_delta);
static int con_message_too_long_to_fit_discards_bytes(struct con *conp, int tn);
static int con_message_too_long_to_fit_doesnt_discard_bytes(struct con *conp,
							    int tn);
static int con_receive_waits_for_message(struct con *conp, int tn);
static int con_receiver_doesnt_wait_for_messages(struct con *conp, int tn);
static int con_receive_returns_what_is_available(struct con *conp, int tn);
static int con_empty_datagram_received_as_empty_datagram(struct con *conp,
							 int tn);

/*
 * test_assert() checks something that should not fail in the test,
 * if it fails the test code is broken, not the system call being tested.
 */

#define	test_assert(expr) 						\
	do {								\
		if (! (expr)) {						\
			tst_res(TBROK, "%s", #expr);			\
			exit(1);					\
		}							\
	} while (0)

/*
 * ensure() tests something that shouldn't fail
 *
 * Test cases use ensure(expr) to indicate that an expression is expected
 * to be true.  When the test cases are run in a verbose manner, the ensure
 * uses produce a trace of the conditions being tested, making it easier
 * to understand what the test cases are verifying.
 * 
 * Messages are not shown unless an expected condition is not true, but if
 * the level of verbosity is high enough, then the non-failing ensure()
 * expressions are shown too, this is just for debug-ability of the tests,
 * in this case the ensure() serve as a trace.
 *
 * A test, made out of multiple ensure() calls is expected not to change the
 * value of ensure_failures.
 */

int ensure_failures;

#define ensure(test_number, expr)					    \
	((expr) ? (test_verbose < 2 ? (void)0 :				    \
		   tst_res(TINFO, "%s(): true: %s", __FUNCTION__, #expr))   \
		: (++ensure_failures,					    \
		   tst_res(TINFO, "test_number = %d: %s(): false: %s",	    \
			   test_number, __FUNCTION__, #expr)))

/*
 *	Leaf test variations, combined into multi-message tests at run time.
 */

struct call call_tests[] = {
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 1},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 2},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 3},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= -2,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= -1,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 0,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 1,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks_peeking,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 2,
	 .call_iov_max_relative = true},

	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 1},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 2},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 3},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= -2,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= -1,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 0,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 1,
	 .call_iov_max_relative = true},
	{.call_test	= call_receive_iovec_boundary_checks,
	 .call_coninit	= con_init_socketpair,
	 .call_niov	= 2,
	 .call_iov_max_relative = true},

	{.call_test	= call_empty_datagram_received_as_empty_datagram,
	 .call_coninit	= con_init_dgram},
	{.call_test	= call_message_too_long_to_fit_discards_bytes,
	 .call_coninit	= con_init_dgram},

	{.call_test	= call_message_too_long_to_fit_doesnt_discard_bytes,
	 .call_coninit	= con_init_seqpacket},
	{.call_test	= call_receive_waits_for_message,
	 .call_coninit	= con_init_seqpacket},
	{.call_test	= call_receiver_doesnt_wait_for_messages,
	 .call_coninit	= con_init_seqpacket},
	{.call_test	= call_receive_returns_what_is_available,
	 .call_coninit	= con_init_seqpacket},

	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= true,
	 .call_controllen_delta	= 0},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= false,
	 .call_controllen_delta	= 0},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= true,
	 .call_controllen_delta	= 0},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= false,
	 .call_controllen_delta	= 0},

	/*
	 * struct ctlmsgfd (with 64 bit size_t) gets an extra int of padding
	 * missing bytes then has to be sizeof(size_t) not sizeof(int),
	 * otherwise there is still space for a sizeof(int) sized fd.
	 */

	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= true,
	 .call_controllen_delta	= -sizeof(size_t)},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= false,
	 .call_controllen_delta	= -sizeof(size_t)},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= true,
	 .call_controllen_delta	= -sizeof(size_t)},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= false,
	 .call_controllen_delta	= -sizeof(size_t)},

	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= true,
	 .call_controllen_delta	= -(ssize_t) (2 * sizeof(size_t))},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= false,
	 .call_controllen_delta	= -(ssize_t) (2 * sizeof(size_t))},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= true,
	 .call_controllen_delta	= -(ssize_t) (2 * sizeof(size_t))},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= false,
	 .call_controllen_delta	= -(ssize_t) (2 * sizeof(size_t))},

	/*
	 * Just one byte in the underlying struct cmsghdr.
	 */

	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= true,
	 .call_controllen_delta	= -(ssize_t) (sizeof(struct ctlmsgfd) - 1)},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= 0,
	 .call_some_data	= false,
	 .call_controllen_delta	= -(ssize_t) (sizeof(struct ctlmsgfd) - 1)},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= true,
	 .call_controllen_delta	= -(ssize_t) (sizeof(struct ctlmsgfd) - 1)},
	{.call_test		= call_receive_file_descriptor,
	 .call_coninit		= con_init_socketpair,
	 .call_cloexec_flag	= MSG_CMSG_CLOEXEC,
	 .call_some_data	= false,
	 .call_controllen_delta	= -(ssize_t) (sizeof(struct ctlmsgfd) - 1)},
};

#define N_TESTS		(sizeof(call_tests) / sizeof(call_tests[0]))
#define	call_tests_end	(&call_tests[N_TESTS])

int test_verbose = 0;	/* 1 a bit verbose, 2 more verbose */
char *prog_name;
bool run_sctp_tests = RUN_SCTP_TESTS;

/*
 *	Test functions.
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

static int con_receive_iovec_boundary_checks(struct con *conp,
					     int tn, size_t niov)
{
	size_t iov_max = iovec_max();
	int salt = tn + subtest_number();	/* salt test data */

	int client_data[niov];
	struct iovec client_iov[niov];
	size_t i;
	for (i = 0; i < niov; ++i) {
		client_data[i] = i + salt;
		iovec_init(&client_iov[i], &client_data[i],
			   sizeof(client_data[0]));
	}

	int server_data[niov];
	struct iovec server_iov[niov];
	for (i = 0; i < niov; ++i) {
		server_data[i] = 0;
		iovec_init(&server_iov[i], &server_data[i],
			   sizeof(server_data[0]));
	}

	struct msghdr client_send;
	msghdr_init(&client_send, NULL, client_iov, niov, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	struct msghdr server_recv;
	msghdr_init(&server_recv, NULL, server_iov, niov, 0);

	struct recvmsg_call rmc;
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

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_iovec_boundary_checks(niov = %d)"
			" // iov_max = %d",
			(int) niov, (int) iovec_max());
	return 0;
}

/*
 * Various boundary tests related to use of more than one iovec,
 * peek at the data first, then receive it
 */

static int con_receive_iovec_boundary_checks_peeking(struct con *conp,
						     int tn, size_t niov)
{
	size_t iov_max = iovec_max();

	int client_data[niov];
	struct iovec client_iov[niov];
	size_t i;
	for (i = 0; i < niov; ++i) {
		client_data[i] = i;
		iovec_init(&client_iov[i], &client_data[i],
			   sizeof(client_data[0]));
	}

	int server_data[niov];
	struct iovec server_iov[niov];
	for (i = 0; i < niov; ++i) {
		server_data[i] = 0;
		iovec_init(&server_iov[i], &server_data[i],
			   sizeof(server_data[0]));
	}

	struct msghdr client_send;
	msghdr_init(&client_send, NULL, client_iov, niov, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	struct msghdr server_recv;
	msghdr_init(&server_recv, NULL, server_iov, niov, 0);

	thread_call(&smc.smc_thread_call, (void *(*)(void *)) sendmsg_call);
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
	} else {
		if (smc.smc_value < 0)
			tst_res(TBROK, "sendmsg");
		if (received < 0)
			tst_res(TBROK, "recvmsg");
		ensure(tn, (size_t) received == sizeof(client_data));
		ensure(tn, mem_is_equal(client_data, server_data, received));
		errno = TEST_HAS_MULTIPLE_RECVMSG;
	}

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_iovec_boundary_checks_peeking(niov"
			" = %d) // iov_max = %d",
			(int) niov, (int) iovec_max());
	return errno;;
}

/*
 * Test that file descriptor is received over a unix domain socket.
 * Variations to test the close on exec flag, whether some amount of
 * data is to be sent as regular data, and missing bytes of space for
 * the control message to receive the file descriptor.  The missing
 * bytes are expressed as a (signed) ssize_t controllen_delta.  Passing
 * in struct cmsghdr requires a negative delta to be small enough so as to
 * actually cause trancation of the space for an int sized file descriptor,
 * a negative controllen_delta must be <= -(ssize_t)(sizeof(size_t)).
 */

static int con_receive_file_descriptor(struct con *conp, int tn,
				       int cloexec_flag, bool some_data,
				       ssize_t controllen_delta)
{
	test_assert(controllen_delta >= 0 ||
	       controllen_delta <= -(ssize_t) (sizeof(size_t)));
	test_assert(cloexec_flag == 0 || cloexec_flag == MSG_CMSG_CLOEXEC);

	int pipefd[2];
	SAFE_PIPE(pipefd);	/* something to send that's easy to check */
	int pipe_read = pipefd[0];
	int pipe_write = pipefd[1];

	struct ctlmsgfd client_ctlmsgfd;
	ctlmsgfd_init(&client_ctlmsgfd, pipe_write);

	struct iovec client_iov;
	char server_data = 0;
	iovec_init(&client_iov, "m", 1);
	struct iovec server_iov;
	iovec_init(&server_iov, &server_data, 1);

	struct iovec *client_iovp = NULL;
	struct iovec *server_iovp = NULL;
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

	struct msghdr client_send;
	msghdr_init_with_control(&client_send, NULL, client_iovp, client_iovlen,
				&client_ctlmsgfd, sizeof(client_ctlmsgfd), 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	struct ctlmsgfd server_ctlmsgfd;
	ctlmsgfd_init_to_zero(&server_ctlmsgfd);

	struct msghdr server_recv;
	msghdr_init_with_control(&server_recv, NULL, server_iovp, server_iovlen,
				&server_ctlmsgfd,
				(size_t) ((ssize_t) sizeof(server_ctlmsgfd)
					  + controllen_delta),
				0);

	struct recvmsg_call rmc;
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
		       sizeof(struct ctlmsgfd));
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

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_file_descriptor("
			"cloexec_flag = %s, some_data = %d, "
			"controllen_delta = %ld)",
			(cloexec_flag & MSG_CMSG_CLOEXEC) ?
				"MSG_CMSG_CLOEXEC" : "0",
			some_data,
			(long) controllen_delta);
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
 * The udp(7) protocol discards the excess bytes.
 *
 * The test sends a 16 byte message, but receives it with an 8 byte buffer,
 * a second recvmsg(2) is done to see if the next 8 bytes are of a 2nd
 * identical 16 byte packet sent or if they belong to the first packet.
 */

static int con_message_too_long_to_fit_discards_bytes(struct con *conp, int tn)
{
	size_t total = 16;
	test_assert(total % 2 == 0);			/* must be even */
	size_t half = total / 2;

	struct iovec client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	test_assert(strlen(client_iov.iov_base) == total);

	struct msghdr client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[half + 1];	/* client sends total, receive half */
	memset(&server_data, 0, half);	/* 1 byte of space for GUARD_CHAR */
	server_data[half] = GUARD_CHAR;
	struct iovec server_iov;
	iovec_init(&server_iov, server_data, half);

	struct sockaddr_in client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	struct msghdr server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	thread_call(&smc.smc_thread_call, (void *(*)(void *)) sendmsg_call);
	ssize_t received = recvmsg(conp->con_server, &server_recv, 0);
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		tst_res(TBROK, "recvmsg");

	ensure(tn, (size_t) received == half);
	ensure(tn, server_data[half] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, half));
	ensure(tn, server_recv.msg_flags & MSG_TRUNC);

	/*
	 * Send the same data in a second message from client, should
	 * receive the same data, not the second half of the first
	 * message, meaning then the excess bytes are being discarded.
	 */

	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	thread_call(&smc.smc_thread_call, (void *(*)(void *)) sendmsg_call);
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

	ensure(tn, (size_t) received == total);
	ensure(tn, server_data[half] == GUARD_CHAR);
	ensure(tn, ! (server_recv.msg_flags & MSG_EOR));
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, half));

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_message_too_long_to_fit_discards_bytes");
	return TEST_HAS_MULTIPLE_RECVMSG;
}

/*
 * To be able to test this specification in the recvmsg(2) man page:
 *
 *	"If a message is too long to fit in the supplied buffer,
 *	 excess bytes may be discarded depending on the type of
 *	 socket the message is received from."
 *
 * The sctp(7) protocol does not discard the excess bytes.
 *
 * Tests for MSG_EOR being set together with the non-discarded bytes.
 * The test sends a 16 byte message, but receives it with an 8 byte buffer,
 * a second recvmsg(2) is done to see if the next 8 bytes are of a 2nd
 * identical 16 byte packet sent or if they belong to the first packet.
 */

static int con_message_too_long_to_fit_doesnt_discard_bytes(struct con *conp,
							    int tn)
{
	size_t total = 16;
	test_assert(total % 2 == 0);			/* must be even */
	size_t half = total / 2;

	struct iovec client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	test_assert(strlen(client_iov.iov_base) == total);

	struct msghdr client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[half + 1];	/* client sends total, receive half */
	memset(&server_data, 0, half);	/* 1 byte of space for GUARD_CHAR */
	server_data[half] = GUARD_CHAR;
	struct iovec server_iov;
	iovec_init(&server_iov, server_data, half);

	struct sockaddr_in client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	struct msghdr server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	thread_call(&smc.smc_thread_call, (void *(*)(void *)) sendmsg_call);
	ssize_t received = recvmsg(conp->con_server, &server_recv, 0);
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		tst_res(TBROK, "recvmsg");

	ensure(tn, (size_t) received == half);
	ensure(tn, server_data[half] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, half));
	ensure(tn, server_recv.msg_flags == 0);

	/*
	 * Send the same data in a second message from client, if it
	 * receives the second half of the first message, then they are not
	 * being discarded and MSG_EOR indicates the end of the first record.
	 */

	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	thread_call(&smc.smc_thread_call, (void *(*)(void *)) sendmsg_call);
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

	ensure(tn, (size_t) received == half);
	ensure(tn, server_data[half] == GUARD_CHAR);
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base + half, half));

	if (test_verbose || subtest_show())
		tst_res(TINFO,
			"con_message_too_long_to_fit_doesnt_discard_bytes");
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

static int con_receive_waits_for_message(struct con *conp, int tn)
{
	size_t total = 16;
	struct iovec client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	test_assert(strlen(client_iov.iov_base) == total);

	struct msghdr client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[total + 1];	
	memset(&server_data, 0, total);	/* 1 byte of space for GUARD_CHAR */
	server_data[total] = GUARD_CHAR;
	struct iovec server_iov;
	iovec_init(&server_iov, server_data, total);

	struct sockaddr_in client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	struct msghdr server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	struct recvmsg_call rmc;
	recvmsg_call_init(&rmc, &server_recv, 0);

	con_add_send_recv_calls(conp, &smc, &rmc, true);

	ssize_t received = rmc.rmc_value;
	if (received < 0)
		tst_res(TBROK, "recvmsg or recvmmsg");

	ensure(tn, (size_t) received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_waits_for_message()");
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

static int con_receiver_doesnt_wait_for_messages(struct con *conp, int tn)
{
	size_t total = 16;
	struct iovec client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	test_assert(strlen(client_iov.iov_base) == total);

	struct msghdr client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[total + 1];	
	memset(&server_data, 0, total);	/* 1 byte of space for GUARD_CHAR */
	server_data[total] = GUARD_CHAR;
	struct iovec server_iov;
	iovec_init(&server_iov, server_data, total);

	struct sockaddr_in client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	struct msghdr server_recv;
	memset(&server_recv, 0, sizeof(server_recv));
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	thread_call(&smc.smc_thread_call,
		    (void *(*)(void *)) sendmsg_call_after_sleep);
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

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receiver_doesnt_wait_for_messages()");
	return TEST_HAS_MULTIPLE_RECVMSG;
}

/*
 * recvmsg(2):
 *
 *	"The receive calls normally return any data available,
 *	 up to the requested amount, rather than waiting for
 *	 receipt of the full amount requested."
 */

static int con_receive_returns_what_is_available(struct con *conp, int tn)
{
	size_t total = 8;
	struct iovec client_iov;
	iovec_init(&client_iov, "01234567", total);
	test_assert(strlen(client_iov.iov_base) == total);

	struct msghdr client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	size_t twice = 2 * total;
	char server_data[twice + 1];	/* client sends total, want twice */
	memset(&server_data, 0, twice);	/* 1 byte of space for GUARD_CHAR */
	server_data[total] = GUARD_CHAR;
	server_data[twice] = GUARD_CHAR;	/* set two guards */
	struct iovec server_iov;
	iovec_init(&server_iov, server_data, twice);

	struct sockaddr_in client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	struct msghdr server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	struct recvmsg_call rmc;
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

	if (test_verbose || subtest_show())
		tst_res(TINFO, "con_receive_returns_what_is_available()");
	return 0;
}

/*
 * recvmsg(2):
 *
 *	"Datagram sockets in various domains (e.g., the UNIX and Internet
 *	 domains) permit zero-length datagrams. When such a datagram is
 *	 received, the return value is 0."
 */

static int con_empty_datagram_received_as_empty_datagram(struct con *conp,
							 int tn)
{
	struct iovec client_iov;
	iovec_init(&client_iov, "", 0);

	struct msghdr client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	struct sendmsg_call smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[1];		/* client sends nothing */
	server_data[0] = GUARD_CHAR;	/* set guard */
	struct iovec server_iov;
	iovec_init(&server_iov, server_data, 1);

	struct sockaddr_in client_sin;
	sockaddr_in_init_to_zero(&client_sin);
	struct msghdr server_recv;
	msghdr_init(&server_recv, &client_sin, &server_iov, 1, 0);

	struct recvmsg_call rmc;
	recvmsg_call_init(&rmc, &server_recv, 0);

	con_add_send_recv_calls(conp, &smc, &rmc, true);

	ssize_t received = rmc.rmc_value;
	if (received < 0)
		tst_res(TBROK, "recvmsg or recvmmsg");

	ensure(tn, received == 0);
	ensure(tn, server_data[0] == GUARD_CHAR);

	if (test_verbose || subtest_show())
		tst_res(TINFO,
			"con_empty_datagram_received_as_empty_datagram()");
	return TEST_NEEDS_SLEEP_IN_SENDER;
}

/*
 *	The test functions above have variations based on their arguments.
 *
 *	The variations are wrapped with specific argument values into objects
 *	that derive from struct call, the wrapped objects don't change so they
 *	can be used to repeat a test multiple times for a multi-message call.
 *
 */

static void tests_run(void)
{
	tests_run_individually();
	tests_run_all_socketpair();
	if (run_sctp_tests) {
		tests_run_all_sock_sepacket();
		tests_run_all_sock_sepacket_with_n_repeat();
	}
	tests_run_all_ocloexec_flag();
}

/*
 *	Test selection and combination into multi-message tests.
 */

static void tests_setup(void)
{
	size_t iov_max = iovec_max();
	struct call *c;
	for (c = call_tests; c < call_tests_end; ++c)
		if (c->call_iov_max_relative)
			c->call_niov += iov_max;
}

static int tests_select(void (*coninit)(struct con *),
			struct call *chosen[], int error)
{
	int n = 0;
	struct call *c;
	for (c = call_tests; c < call_tests_end; ++c)
		if (c->call_coninit == coninit && c->call_errno == error) {
			if (!run_sctp_tests &&
			    c->call_coninit == con_init_seqpacket)
				continue;
			*chosen++ = c;
			++n;
		}
	return n;
}

static void tests_run_individually(void)
{
	tst_res(TINFO, "Running %d individual sendmsg(2) tests:\n",
		(int) N_TESTS);
	struct call *c;
	for (c = call_tests; c < call_tests_end; ++c) {
		if (!run_sctp_tests && c->call_coninit == con_init_seqpacket)
			continue;
		struct con con;
		c->call_coninit(&con);
		c->call_errno = call_go(c, &con);
		con_deinit(&con);
	}
}

static void tests_run_all_socketpair(void)
{
	struct con con;
	int error;
	struct call *callp;
	size_t max_tests = iovec_max();
	test_assert(max_tests > N_TESTS);
	struct call *call_tests_selected[max_tests + 1];
	struct sendmsg_call *smc_vec[max_tests];
	struct recvmsg_call *rmc_vec[max_tests];

	/*
	 * Select all the socketpair() based test cases a test array so
	 * they can be used together in single recvmmsg() multi-message test
	 * that uses the same connection.
	 */

	int n_selected = tests_select(con_init_socketpair,
				      call_tests_selected, 0);

	/*
	 * Call them individually sharing the connection one after the 
	 * other to ensure there are no dependencies, for example left-
	 * over data leakage left in the socket between individual tests
	 * that might break the tests
	 */

	tst_res(TINFO, "Running %d non-error socketpair() "
		"tests sharing a connection:\n", (int) n_selected);
	con_init_socketpair(&con);
	int i;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		error = call_go(callp, &con);
		test_assert(!error);
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
	 * recurse all the struct sendmsg_call and struct recvmsg_call have been
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
		tst_res(TINFO, "Running %d non-error socketpair() tests"
			" with a single recvmmsg(2) call and %s:\n",
			n_selected,
			sendmulti ? "a single sendmmsg(2) call"
				  : "multiple sendmsg(2) calls");
		con_make_vectored(&con, sendmulti > 0, n_selected,
				  call_tests_selected, smc_vec, rmc_vec);
		callp = call_tests_selected[0];
		error = call_go(callp, &con);
		test_assert(!error);					
		con_deinit(&con);
	}

	/*
	 * Run the recvmmsg(2) test from above with sleeps in the
	 * sender and MSG_WAITFORONE in the receiver, the receiver
	 * keeps on on re-trying until they are all received.
	 */

	tst_res(TINFO, "Running %d non-error socketpair() tests with as"
		" many MSG_WAITFORONE recvmmsg(2) calls as needed and"
		" individual sleeping sendmsg(2) calls:\n", n_selected);
	con_init_socketpair(&con);
	con_make_vectored(&con, false, n_selected,
			  call_tests_selected, smc_vec, rmc_vec);
	con_make_waitforone_tests(&con);
	callp = call_tests_selected[0];
	error = call_go(callp, &con);
	test_assert(!error);					
	con_deinit(&con);

	/*
	 * Run the recvmmsg(2) test from above with sleeps in the
	 * sender and timeouts in the receiver, the receiver keeps
	 * on on re-trying until they are all received.
	 */

	tst_res(TINFO, "Running %d non-error socketpair() tests with as"
		" many recvmmsg(2) calls with a timeout as needed and"
		" individual sleeping sendmsg(2) calls:\n", n_selected);
	con_init_socketpair(&con);
	con_make_vectored(&con, false, n_selected,
			  call_tests_selected, smc_vec, rmc_vec);
	con_make_timeout_tests(&con);
	callp = call_tests_selected[0];
	error = call_go(callp, &con);
	test_assert(!error);					
	con_deinit(&con);
}

static void tests_run_all_ocloexec_flag(void)
{
	struct con con;
	int error;
	size_t max_tests = iovec_max();
	test_assert(max_tests > N_TESTS);
	struct call *call_tests_selected[max_tests + 1];
	struct sendmsg_call *smc_vec[max_tests];
	struct recvmsg_call *rmc_vec[max_tests];

	/*
	 * Get the socketpair tests that returned TEST_CLOEXEC_FLAG
	 * and run them sequentially to ensure there are no issues with
	 * the connection sharing.
	 */

	int n_selected = tests_select(con_init_socketpair,
				      call_tests_selected,
				      TEST_CLOEXEC_FLAG);
	tst_res(TINFO, "Running %d TEST_CLOEXEC_FLAG socketpair() "
		"tests sharing a connection:\n", n_selected);
	con_init_socketpair(&con);
	int i;
	struct call *callp;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int error = call_go(callp, &con);
		test_assert(error == TEST_CLOEXEC_FLAG);
	}
	con_deinit(&con);

	/*
	 * Now run them with recvmmsg(2)
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
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
		test_assert(error == TEST_CLOEXEC_FLAG);
		con_deinit(&con);
	}
}

static void tests_run_all_sock_sepacket(void)
{
	struct con con;
	int error;
	size_t max_tests = iovec_max();
	test_assert(max_tests > N_TESTS);
	struct call *call_tests_selected[max_tests + 1];
	struct sendmsg_call *smc_vec[max_tests];
	struct recvmsg_call *rmc_vec[max_tests];

	/*
	 * Select the SOCK_SEQPACKET tests, run them sequentially sharing
	 * the connection to ensure there are no issues sharing it.
	 */

	int n_selected = tests_select(con_init_seqpacket,
				      call_tests_selected, 0);
	tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
		"tests sharing a connection:\n", n_selected);
	con_init_seqpacket(&con);
	int i;
	struct call *callp;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		error = call_go(callp, &con);
		test_assert(!error);
	}
	con_deinit(&con);

	/*
	 * Run the SOCK_SEQPACKET tests with multiple sendmsg(2) calls
	 * (or a single sendmmsg(2) call) and recvmmsg(2).
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
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
		test_assert(!error);					
		con_deinit(&con);
	}
}

#define N_REPEAT 4

static void tests_run_all_sock_sepacket_with_n_repeat(void)
{
	struct con con;
	int error;
	size_t iov_max = iovec_max();
	size_t max_tests = iov_max;
	test_assert(max_tests > N_TESTS);
	struct call *call_tests_selected[max_tests + 1];
	struct sendmsg_call *smc_vec[max_tests];
	struct recvmsg_call *rmc_vec[max_tests];

	/*
	 * Get the SOCK_SEQPACKET tests again, repeat them N_REPEAT times
	 * and run them sequentially to ensure there are no issues with the
	 * connection sharing.
	 */

	int n_selected = tests_select(con_init_seqpacket,
				      call_tests_selected, 0);
	test_assert(N_REPEAT * n_selected < (int) N_TESTS);
	int i;
	struct call *callp;
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int skip, r;
		for (skip = n_selected, r = 0; r < N_REPEAT;
		     ++r, skip += n_selected)
			call_tests_selected[i + skip] = callp;
	}

	n_selected *= N_REPEAT;
	tst_res(TINFO, "Running %d non-error SOCK_SEQPACKET "
		"tests sharing a connection (N_REPEAT = %d):\n",
		n_selected, N_REPEAT);
	con_init_seqpacket(&con);
	for (i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		error = call_go(callp, &con);
		test_assert(!error);
	}
	con_deinit(&con);

	/*
	 * Now run them with recvmmsg(2)
	 */

	int sendmulti;
	for (sendmulti = 0; sendmulti <= 1; ++sendmulti) {
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
		test_assert(!error);					
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
		test_assert(!error);					
		con_deinit(&con);
	}
}

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

static void tst_parse_opt(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	tst_parse_opt(argc, argv);
	tests_setup();
	tests_run();
	exit(ensure_failures > 0 ? 1 : 0);
}

static void tst_parse_opt(int argc, char *argv[])
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

/*
 *	Utility functions and mechanism for test combinations and for
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

static void con_init_base(struct con *conp, int client, int server,
			  int type, struct sockaddr_in *sinp)
{
	conp->con_type = type;
	conp->con_client = client;
	conp->con_server = server;
	if (!sinp)
		memset(&conp->con_server_sin, 0 , sizeof(conp->con_server_sin));
	else
		conp->con_server_sin = *sinp;
}

static void con_init(struct con *conp, int client, int server,
		     int type, struct sockaddr_in *sinp)
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

static void con_make_vectored(struct con *conp, bool sendmulti,
			      size_t n, struct call **call_vec,
			      struct sendmsg_call **smc_vec,
			      struct recvmsg_call **rmc_vec)
{
	test_assert(n >= 1 && call_vec && smc_vec && rmc_vec);
	test_assert(conp->con_client >= 0 &&
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

static void con_make_waitforone_tests(struct con *conp)
{
	test_assert(conp->con_call_vec);
	test_assert(!conp->con_call_sendmmsg);
	conp->con_timeout = true;
}

static void con_make_timeout_tests(struct con *conp)
{
	test_assert(conp->con_call_vec);
	test_assert(!conp->con_call_sendmmsg);
	conp->con_waitforone = true;
}

static void con_add_send_recv_calls_vec(struct con *conp,
					struct sendmsg_call *smcp,
					struct recvmsg_call *rmcp,
					bool sender_sleeps)
{
	test_assert(conp->con_call_end < conp->con_call_n);
	size_t i = conp->con_call_end;

	/*
	 * All senders must either: need to sleep or must not sleep.
	 */

	if (i == 0)
		conp->con_sender_sleeps = sender_sleeps;
	else
		test_assert(conp->con_sender_sleeps == sender_sleeps);
	conp->con_smc_vec[i] = smcp;
	conp->con_rmc_vec[i] = rmcp;

	++i;
	conp->con_call_end = i;
	if (i == conp->con_call_n)
		con_do_multi_send_recv(conp);
	else {
		subtest_nest();
		struct call *callp = conp->con_call_vec[i];
		int error = call_go(callp, conp);
		test_assert(!errno_value_is_error(error));
		subtest_unnest();
	}
}

static void *sendmmsg_call(struct sendmmsg_call *smmcp)
{
	errno = 0;

	int sockfd = smmcp->smmc_sockfd;
	int vlen = smmcp->smmc_vlen;
	struct mmsghdr *smm = smmcp->smmc_smm;
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
			test_assert(value >= 0);
			smm[nsent].msg_len = value;
		}
	}

	smmcp->smmc_value = nsent;
	smmcp->smmc_errno = errno;
	return NULL;
}

static void con_sendmmsg_recvmmsg(struct con *conp, size_t n,
				  int sflags, struct mmsghdr *smm,
				  int rflags, struct mmsghdr *rmm)
{
	bool waitforone = conp->con_waitforone;
	bool timeout = conp->con_timeout;
	test_assert(!waitforone || !timeout);

	struct sendmmsg_call smmc;
	sendmmsg_init(&smmc, conp->con_client, (unsigned) n,
		      smm, sflags, conp->con_call_sendmmsg,
		      waitforone || timeout || conp->con_sender_sleeps);

	thread_call(&smmc.smmc_thread_call, (void *(*)(void *)) sendmmsg_call);

	int nrecv;
	if (!waitforone && !timeout) {
		nrecv = recvmmsg(conp->con_server, rmm, n, rflags, NULL);
		test_assert((size_t) nrecv == n);
	} else {
		struct timespec ts, *tsp = NULL;
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
				test_assert(nrecv > 0);
			if (timeout && nrecv == -1) {
				nrecv = 0;
				test_assert(errno == ETIMEDOUT ||
				       errno == EAGAIN ||
				       errno == EWOULDBLOCK);
			}
			nleft -= nrecv;
			rmm += nrecv;
		} while (nleft > 0);
	}

	thread_join(&smmc.smmc_thread_call);

	int nsent = smmc.smmc_value;
	test_assert((size_t) nsent == n);
}

static void con_do_multi_send_recv(struct con *conp)
{
	test_assert(conp->con_call_n >= 1 && conp->con_call_n <= iovec_max());
	size_t n = conp->con_call_n;
	struct mmsghdr smm[n];
	struct mmsghdr rmm[n];

	/*
	 * Gather the arguments from all the struct sendmsg_call and the
	 * struct recvmsg_call into the smm[] and rmm[] vectors for the
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
		test_assert(conp->con_smc_vec[i]->smc_flags == sflags);
		test_assert(conp->con_rmc_vec[i]->rmc_flags == rflags);
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

static void con_init_socketpair(struct con *conp)
{
	int pair[2];
	SAFE_SOCKETPAIR(AF_UNIX, SOCK_SEQPACKET, 0, pair);
	con_init(conp, pair[0], pair[1], SOCK_SEQPACKET, NULL);
}

static void con_init_seqpacket(struct con *conp)
{
	int server = SAFE_SOCKET(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	struct sockaddr_in sin;
	sockaddr_in_init(&sin, AF_INET, TEST_PORT, TEST_IP);
	SAFE_BIND(server, (struct sockaddr *) &sin, sizeof(sin));
	int client = SAFE_SOCKET(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	SAFE_LISTEN(server, 1);
	con_init(conp, client, server, SOCK_SEQPACKET, &sin);
}

static void con_init_dgram(struct con *conp)
{
	int server = SAFE_SOCKET(PF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sin;
	sockaddr_in_init(&sin, AF_INET, TEST_PORT, TEST_IP);
	SAFE_BIND(server, (struct sockaddr *) &sin, sizeof(sin));
	int client = SAFE_SOCKET(PF_INET, SOCK_DGRAM, 0);
	con_init(conp, client, server, SOCK_DGRAM, &sin);
}

static void con_deinit(struct con *conp)
{
	close(conp->con_client);
	close(conp->con_server);
}

static void con_add_send_recv_calls(struct con *conp, struct sendmsg_call *smcp,
				    struct recvmsg_call *rmcp,
				    bool sender_sleeps)
{
	if (conp->con_call_n > 0) {
		con_add_send_recv_calls_vec(conp, smcp, rmcp, sender_sleeps);
		return;
	}

	thread_call(&smcp->smc_thread_call, (void *(*)(void *))
		    (sender_sleeps ? sendmsg_call_after_sleep : sendmsg_call));
	struct timespec pretv, postv;
	if (sender_sleeps)
		tst_fzsync_time(&pretv);
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
		tst_fzsync_time(&postv);
		unsigned long long pre = pretv.tv_sec * 1000 * 1000 * 1000;
		pre += pretv.tv_nsec;
		unsigned long long pos = postv.tv_sec * 1000 * 1000 * 1000;
		pos += postv.tv_nsec;
		test_assert((pos - pre) > (SENDMSG_SLEEP_MS * 1000) / 2);
	}

	rmcp->rmc_value = val;
	thread_join(&smcp->smc_thread_call);
}

/*
 *	Miscellaneous supporting code is in this section.
 */

static int errno_value_is_error(int e)
{
	return e > 0;  /* negative values are not errors, see non_error_errno */
}

static void sleep_ms(long ms)
{
	test_assert(ms < 1000);
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ms * 1000 * 1000;
	nanosleep(&ts, NULL);
}

static int test_number_get(void)
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

static int subtest_level = 1;

static int subtest_number(void)
{
	return subtest_level;
}

static void subtest_nest(void)
{
	++subtest_level;
}

static void subtest_unnest(void)
{
	--subtest_level;
}

static bool subtest_show(void)
{
	/*
	 *	Some tests involve too many subtests, this limits the output.
	 */
	return subtest_level <= 20;
}

/*
 * compare that memory is equal, makes the output of various ensure() nicer
 */

static bool mem_is_equal(void *a, void *b, size_t size)
{
	return !memcmp(a, b, size);
}

static bool mem_is_zero(void *a, size_t size)
{
	unsigned char set = 0;
	unsigned char *p = a;
	unsigned char *endp = p + size;
	while (p < endp) set |= *p++;
	return !set;
}

static void thread_call(struct thread_call *tcp, void *(*func)(void *))
{
	SAFE_PTHREAD_CREATE(&tcp->tc_thr, NULL, func, tcp);
}

static void thread_join(struct thread_call *tcp)
{
	SAFE_PTHREAD_JOIN(tcp->tc_thr, NULL);
}

static void sendmmsg_init(struct sendmmsg_call *smmcp, int sockfd,
			  unsigned vlen, struct mmsghdr *smm, int flags,
			  bool call_sendmmsg, bool sender_sleeps)
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

static void sockaddr_in_init(struct sockaddr_in *sinp, sa_family_t family,
			     in_port_t port, uint32_t ip)
{
	sockaddr_in_init_to_zero(sinp);
	sinp->sin_family = family;
	sinp->sin_port = htons(port);
	sinp->sin_addr.s_addr = htonl(ip);
}

static void sockaddr_in_init_to_zero(struct sockaddr_in *sinp)
{
	memset(sinp, 0, sizeof(*sinp));
}

static size_t iovec_max(void)
{
	long iov_max = SAFE_SYSCONF(_SC_IOV_MAX);
	test_assert(iov_max > 2);
	return (size_t) iov_max;
}

static void iovec_init(struct iovec *iovecp, void *base, size_t len)
{
	iovecp->iov_base = base;
        iovecp->iov_len = len;
}

static void msghdr_init(struct msghdr *msgp, struct sockaddr_in *sinp,
			struct iovec *iovecp, size_t iovlen, int flags)
{
	msghdr_init_with_control(msgp, sinp, iovecp, iovlen, NULL, 0, flags);
}

static void msghdr_init_with_control(struct msghdr *msgp,
				     struct sockaddr_in *sinp,
				     struct iovec *iovecp, size_t iovlen,
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

static void mmsghdr_init(struct mmsghdr *mmsgp, struct msghdr *msgp)
{
	mmsgp->msg_hdr = *msgp;
	mmsgp->msg_len = 0;
}

static void recvmsg_call_init(struct recvmsg_call *rmcp,
			      struct msghdr *msgp, int flags)
{
	rmcp->rmc_value = INVALID_SYSCALL_VALUE;
	rmcp->rmc_errno = 0;
	rmcp->rmc_msg = msgp;
	rmcp->rmc_flags = flags;
}

static void sendmsg_call_init(struct sendmsg_call *smcp, int sockfd,
			      struct msghdr *msgp, int flags)
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

static void *sendmsg_call(struct sendmsg_call *smcp)
{
	errno = 0;
	smcp->smc_value = sendmsg(smcp->smc_sockfd, smcp->smc_msg,
				  smcp->smc_flags);
	smcp->smc_errno = errno;
	return NULL;
}

static void *sendmsg_call_after_sleep(struct sendmsg_call *smcp)
{
	sleep_ms(SENDMSG_SLEEP_MS);
	return sendmsg_call(smcp);
}

static void ctlmsgfd_init(struct ctlmsgfd *cmfp, int fd)
{
	memset(cmfp, 0, sizeof(*cmfp));
	cmfp->cmf_cmsg.cmsg_len = sizeof(*cmfp);
        cmfp->cmf_cmsg.cmsg_level = SOL_SOCKET;
        cmfp->cmf_cmsg.cmsg_type = SCM_RIGHTS;
	*(int *) CMSG_DATA(&cmfp->cmf_cmsg) = fd;
}

static void ctlmsgfd_init_to_zero(struct ctlmsgfd *cmfp)
{
	memset(cmfp, 0, sizeof(*cmfp));
}

static int ctlmsgfd_get_fd(struct ctlmsgfd *cmfp)
{
	return *(int *) CMSG_DATA(&cmfp->cmf_cmsg);
}

static int call_go(struct call *callp, struct con *conp)
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

static int call_receive_iovec_boundary_checks(struct call *callp,
					      struct con *conp, int tn)
{
	return con_receive_iovec_boundary_checks(conp, tn, callp->call_niov);
}
static int call_receive_iovec_boundary_checks_peeking(struct call *callp,
						      struct con *conp, int tn)
{
	return con_receive_iovec_boundary_checks_peeking(conp, tn,
							 callp->call_niov);
}
static int call_receive_file_descriptor(struct call *callp,
					struct con *conp, int tn)
{
	return con_receive_file_descriptor(conp, tn,
					   callp->call_cloexec_flag,
					   callp->call_some_data,
					   callp->call_controllen_delta);
}
static int call_message_too_long_to_fit_discards_bytes(struct call *callp,
						       struct con *conp,
						       int tn)
{
	(void)(callp);
	return con_message_too_long_to_fit_discards_bytes(conp, tn);
}
static int call_message_too_long_to_fit_doesnt_discard_bytes(struct call *callp,
							     struct con *conp,
							     int tn)
{
	(void)(callp);
	return con_message_too_long_to_fit_doesnt_discard_bytes(conp, tn);
}
static int call_receive_waits_for_message(struct call *callp,
					  struct con *conp, int tn)
{
	(void)(callp);
	return con_receive_waits_for_message(conp, tn);
}
static int call_receiver_doesnt_wait_for_messages(struct call *callp,
						  struct con *conp, int tn)
{
	(void)(callp);
	return con_receiver_doesnt_wait_for_messages(conp, tn);
}
static int call_receive_returns_what_is_available(struct call *callp,
						  struct con *conp, int tn)
{
	(void)(callp);
	return con_receive_returns_what_is_available(conp, tn);
}
static int call_empty_datagram_received_as_empty_datagram(struct call *callp,
							  struct con *conp,
							  int tn)
{
	(void)(callp);
	return con_empty_datagram_received_as_empty_datagram(conp, tn);
}
