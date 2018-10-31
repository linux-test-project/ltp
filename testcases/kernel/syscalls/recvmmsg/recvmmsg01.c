 //  Copyright (c) Google LLC, 2018
 //  SPDX-License-Identifier: GPL-2.0-or-later
 //
 //{

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

// #define RECVMMSG_USE_SCTP
#ifdef RECVMMSG_USE_SCTP
#include <netinet/sctp.h>
#endif

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
// #include "safe_macros.h"

 //}
 // 	Declarations.
 //
 //{	<-- (curly-brace match to move through major sections of this file)

 //  Test cases use ensure(expr) to indicate that an expression is expected
 //  to be true.  Some test cases, depending on its arguments, tests for
 //  the same condition and its opposite.  For easier reading of the output
 //  extra ensure() uses are made to cause useful output to be produced, for
 //  example on both branches of an if-else that has already tested the
 //  condition. For example:
 //
 //	if (server_recv.msg_flags & MSG_EOR) {
 //		ensure(server_recv.msg_flags & MSG_EOR);
 //		...
 //	} else {
 //		ensure(! (server_recv.msg_flags & MSG_EOR));
 //		...
 //	}
 //
 //  This is not sloppy coding, its done purposely, don't delete such code.

 //  contants

enum { GUARD_CHAR = '+' };		// guard character to check overwrites
enum { INVALID_SYSCALL_VALUE = -2 };	// syscalls values are all >= -1
enum { TEST_IP = 0x7F000001 };
enum { TEST_PORT = 6789 };
enum { SENDMSG_SLEEP_MS = 10 };

 //  Must be disjoint with errno values and non-zero, value returned by tests
 //  that do multiple recvmsg() calls internally, or that have a non-zero flag,
 //  or that require special treatment (sender needs to sleep).  Those tests
 //  can not be bundled with other tests into a larger recvmmsg(2) multi-
 //  receive message test, some can only be bundled with their own kind.

enum non_error_errno {
	TEST_HAS_MULTIPLE_RECVMSG = -1,		// can't be bundled at all
	TEST_CLOEXEC_FLAG = -2,			// bundle these together
	TEST_NEEDS_SLEEP_IN_SENDER = -3,	// bundle these together
};

 //  typedefs

typedef struct timeval		timeval_t;
typedef struct timespec		timespec_t;
typedef struct iovec		iovec_t;
typedef struct msghdr		msghdr_t;
typedef struct mmsghdr		mmsghdr_t;
typedef struct sockaddr		sockaddr_t;
typedef struct sockaddr_in	sockaddr_in_t;
typedef struct sctp_initmsg	sctp_initmsg_t;
typedef struct cmsghdr		cmsghdr_t;

typedef void *(*thread_start_t)(void *arg);

typedef struct {
	cmsghdr_t	cmf_cmsg;
	int		cmf_fdspace;	// padding in cmf_cmsg implies fd might
					// not be here, might be in cmf_cmsg!
} ctlmsgfd_t;

typedef struct {
	pthread_t	 tc_thr;
} thread_call_t;

 //  sendmsg(2) call to be sent individually or bundled up with other such
 //  calls with sendmmsg(2) by copying them into an mmsghdr_t array, the
 //  sending is done by a pthread_create(3) created thread

typedef struct {
	thread_call_t	 smc_thread_call;	// must be first: inherits from
	int		 smc_sockfd;
	msghdr_t	*smc_msg;
	int		 smc_flags;
	ssize_t		 smc_value;
	int		 smc_errno;
} sendmsg_call_t;

 //  sendmmsg(2) call, will be done as a single sendmmsg(2) call or vlen
 //  individual sendmsg(2) calls

typedef struct {
	thread_call_t	 smmc_thread_call;	// must be first: inherits from
	int		 smmc_sockfd;
	unsigned	 smmc_vlen;
	mmsghdr_t	*smmc_smm;
	int		 smmc_flags;
	bool		 smmc_call_sendmmsg;
	bool		 smmc_sender_sleeps;
	int		 smmc_value;
	int		 smmc_errno;
} sendmmsg_call_t;

 //  recvmsg(2) call to be received individually or bundled up with other
 //  such calls with recvmmsg(2) by copying them into an mmsghdr_t array

typedef struct {
	ssize_t		 rmc_value;
	int		 rmc_errno;
	msghdr_t	*rmc_msg;
	int		 rmc_flags;
} recvmsg_call_t;

 //  client server connection

typedef struct call_s call_t;

 //  A con_t abstracts a connection between a client and a server, it is used
 //  two different ways:
 //
 //  con_call_vec == NULL && con_call_n == 0 && con_call_sendmmsg == false
 //
 //  - Individual tests encapsulated in a single test function, these are
 //    usually just one sendmsg(2) and one or maybe two sequential recvmsg(2)
 //    calls, for example to peek then get the message or to test non-blocking
 //    then blocking recvmsg, etc.
 //	
 //  con_call_vec != NULL && con_call_n >= 1
 //
 //  - Bundled unrelated tests each encapsulated in their own test function,
 //    all of these tests have to be implemented with a 1 send and 1 recveive,
 //    so that they can all run with a multi-receive message, recvmmsg(2), call.
 //    In that case the con_t is used to hide the bundling of the tests from
 //    each other and the tests, unsupectingly, pass the test vector via the
 //    con_call_vec and con_call_n members.  Note that con_call_n == 1 is used
 //    as a degenerate case to test recvmmsg(2) with a single message. Note
 //    when recvmmsg(2) is used the messages can be sent individually or in
 //    a single sendmmsg(2) for testing multi-message sending. Both ways of
 //    sending are done controlled by con_call_sendmmsg == true to indicate
 //    used of sendmmsg(2), otherwise multiple sendmsg(2) are used.

typedef struct {
	int		  con_client;
	int		  con_server;
	int		  con_type;
	sockaddr_in_t	  con_server_sin;
	bool		  con_call_sendmmsg;
	bool		  con_sender_sleeps;
	bool		  con_waitforone;
	bool		  con_timeout;
	size_t		  con_call_end;	// index of last+1 filled
	size_t		  con_call_n;
	call_t		**con_call_vec;	// points to array of con_call_n
					// call_t pointers to be used in
					// a recvmmsg(2) multi-message test
	sendmsg_call_t	**con_smc_vec;
	recvmsg_call_t	**con_rmc_vec;
} con_t;

typedef int (*calltest_t)(call_t *callp, con_t *conp, int tn);
typedef void (*callconinit_t)(call_t *callp, con_t *conp);

struct call_s {
	calltest_t	call_test;
	callconinit_t	call_con_init;
	int		call_errno;	// set on first run
};

typedef struct {
	call_t	call_base;		// inherits from call_t
	size_t	call_niov;
} call_receive_iovec_boundary_checks_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
	size_t	call_niov;
} call_receive_iovec_boundary_checks_peeking_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
	int	call_cloexec_flag;
	bool	call_some_data;
	ssize_t	call_controllen_delta;
} call_receive_file_descriptor_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
	int	call_type;
} call_message_too_long_to_fit_might_discard_bytes_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
} call_receive_waits_for_message_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
} call_receiver_doesnt_wait_for_messages_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
} call_receive_returns_what_is_available_t;

typedef struct {
	call_t	call_base;		// inherits from call_t
} call_empty_datagram_received_as_empty_datagram_t;

 //  prototypes

void tests_run(void);
void tests_wrap(void);
size_t tests_select(callconinit_t cci, call_t *tests_src[],
		    call_t *tests_dest[], size_t n, int error);

void perror_exit_file_line(const char *s, const char *file,
			   const char *func, int line);
void strerror_exit_file_line(int e, const char *s,
			     const char *file, const char *func, int line);
int errno_value_is_error(int e);

void sleep_ms(long ms);

void thread_call(thread_call_t *tcp, thread_start_t func);
void thread_join(thread_call_t *tcp);

void tst_parse_opt(int argc, char *argv[]);

void print_nest(int test_number);
void print_prefix(int test_number);

int test_number_get(void);

int subtest_number(void);
void subtest_nest(void);
void subtest_unnest(void);

bool mem_is_equal(void *a, void *b, size_t size);
bool mem_is_zero(void *a, size_t size);

void sockaddr_in_init(sockaddr_in_t *sinp, sa_family_t family,
		      in_port_t port, uint32_t ip);
void sockaddr_in_init_to_zero(sockaddr_in_t *sinp);

char *sock_type_to_str(int type);

size_t iovec_max(void);
void iovec_init(iovec_t *iovecp, void *base, size_t len);

void msghdr_init(msghdr_t *msgp, sockaddr_in_t *sinp,
		 iovec_t *iovecp, size_t iovlen, int flags);
void msghdr_init_with_control(msghdr_t *msgp, sockaddr_in_t *sinp,
			      iovec_t *iovecp, size_t iovlen,
			      void *control, size_t controllen,
			      int flags);

void mmsghdr_init(mmsghdr_t *mmsgp, msghdr_t *msgp);

void recvmsg_call_init(recvmsg_call_t *rmcp,
		       msghdr_t *msgp, int flags);

void sendmsg_call_init(sendmsg_call_t *smcp, int sockfd,
		       msghdr_t *msgp, int flags);
void *sendmsg_call(sendmsg_call_t *smcp);
void *sendmsg_call_after_sleep(sendmsg_call_t *smcp);

void sendmmsg_init(sendmmsg_call_t *smmcp, int sockfd, unsigned vlen,
		   mmsghdr_t *smm, int flags, bool call_sendmmsg,
		   bool sender_sleeps);

void ctlmsgfd_init(ctlmsgfd_t *cmfp, int fd);
void ctlmsgfd_init_to_zero(ctlmsgfd_t *cmfp);
int ctlmsgfd_get_fd(ctlmsgfd_t *cmfp);

void con_init(con_t *conp, int client, int server,
	      int type, sockaddr_in_t *sinp);
void con_init_base(con_t *conp, int client, int server,
	      int type, sockaddr_in_t *sinp);
void con_make_vectored(con_t *conp, bool sendmulti,
		       size_t n, call_t **call_vec,
		       sendmsg_call_t **smc_vec, recvmsg_call_t **rmc_vec);
void con_make_waitforone_tests(con_t *conp);
void con_make_timeout_tests(con_t *conp);
void con_init_with_type(con_t *conp, int type);
void con_init_socketpair(con_t *conp);
void con_init_seqpacket(con_t *conp);
void con_init_dgram(con_t *conp);
void con_deinit(con_t *conp);
void con_sendmmsg_recvmmsg(con_t *conp, size_t n,
			   int sflags, mmsghdr_t *smm,
			   int rflags, mmsghdr_t *rmm);
void con_do_multi_send_recv(con_t *conp);
void con_add_send_recv_calls_vec(con_t *conp, sendmsg_call_t *smcp,
				 recvmsg_call_t *rmcp, bool sender_sleeps);
void con_add_send_recv_calls(con_t *conp, sendmsg_call_t *smcp,
			     recvmsg_call_t *rmcp, bool sender_sleeps);

int call_go(call_t *callp, con_t *conp);
void call_do_con_init_socketpair(call_t *callp, con_t *conp);
void call_do_con_init_seqpacket(call_t *callp, con_t *conp);
void call_do_con_init_dgram(call_t *callp, con_t *conp);
void call_base_init(call_t *callp, callconinit_t coninit, calltest_t test);

void call_receive_iovec_boundary_checks_init(
	call_receive_iovec_boundary_checks_t *callp, size_t niov);
int call_receive_iovec_boundary_checks(
	call_receive_iovec_boundary_checks_t *callp, con_t *conp, int tn);

void call_receive_iovec_boundary_checks_peeking_init(
	call_receive_iovec_boundary_checks_peeking_t *callp, size_t niov);
int call_receive_iovec_boundary_checks_peeking(
	call_receive_iovec_boundary_checks_peeking_t *callp,
	con_t *conp, int tn);

void call_receive_file_descriptor_init(
	call_receive_file_descriptor_t *callp,
	int cloexec_flag, bool some_data, ssize_t controllen_delta);
int call_receive_file_descriptor(
	call_receive_file_descriptor_t *callp, con_t *conp, int tn);

void call_message_too_long_to_fit_might_discard_bytes_init(
	call_message_too_long_to_fit_might_discard_bytes_t *callp,
	int type);
int call_message_too_long_to_fit_might_discard_bytes(
	call_message_too_long_to_fit_might_discard_bytes_t *callp,
	con_t *conp, int tn);

void call_empty_datagram_received_as_empty_datagram_init(
	call_empty_datagram_received_as_empty_datagram_t *callp);
int call_empty_datagram_received_as_empty_datagram(
	call_empty_datagram_received_as_empty_datagram_t *callp,
	con_t *conp, int tn);

void call_receive_waits_for_message_init(
	call_receive_waits_for_message_t *callp);
int call_receive_waits_for_message(
	call_receive_waits_for_message_t *callp, con_t *conp, int tn);

void call_receiver_doesnt_wait_for_messages_init(
	call_receiver_doesnt_wait_for_messages_t *callp);
int call_receiver_doesnt_wait_for_messages(
	call_receiver_doesnt_wait_for_messages_t *callp, con_t *conp, int tn);

void call_receive_returns_what_is_available_init(
	call_receive_returns_what_is_available_t *callp);
int call_receive_returns_what_is_available(
	call_receive_returns_what_is_available_t *callp, con_t *conp, int tn);

int con_receive_iovec_boundary_checks(con_t *conp, int tn, size_t niov);
int con_receive_iovec_boundary_checks_peeking(con_t *conp, int tn, size_t niov);
int con_receive_file_descriptor(con_t *conp, int tn, int cloexec_flag,
				bool some_data, ssize_t controllen_delta);
int con_message_too_long_to_fit_might_discard_bytes(con_t *conp, int tn,
						    int type);
int con_receive_waits_for_message(con_t *conp, int tn);
int con_receiver_doesnt_wait_for_messages(con_t *conp, int tn);
int con_receive_returns_what_is_available(con_t *conp, int tn);
int con_empty_datagram_received_as_empty_datagram(con_t *conp, int tn);

 //  misc

char *prog_name;

 //  error reporting and exiting

int test_verbose = 0;	// 0 not verbose, 1 verbose, 2 even more verbose
int ensure_failures;

#define perror_exit(s) \
	perror_exit_file_line(s, __FILE__, __FUNCTION__, __LINE__)

#define strerror_exit(e, s) \
	strerror_exit_file_line(e, s, __FILE__, __FUNCTION__, __LINE__)

 //  ensure() tests something that shouldn't fail, doesn't abort like assert()

#define ensure(test_number, expr)					    \
	((expr) ? (test_verbose < 2 ||			    		    \
		   (print_prefix(test_number),				    \
		    printf("%s(): true: " #expr " \n", __FUNCTION__)))	    \
		: (++ensure_failures,					    \
		   printf("test_number = %d: %s(): false: "		    \
			  #expr " : failed\n",				    \
			  test_number, __FUNCTION__)))

 //}
 //{	Test functions
 //

 //  Various boundary tests related to use of more than one iovec

int con_receive_iovec_boundary_checks(con_t *conp, int tn, size_t niov)
{
	size_t iov_max = iovec_max();
	int salt = tn + subtest_number();	// salt test data

	int client_data[niov];
	iovec_t client_iov[niov];
	for (int i = 0; i < niov; ++i) {
		client_data[i] = i + salt;
		iovec_init(&client_iov[i], &client_data[i],
			   sizeof(client_data[0]));
	}

	int server_data[niov];
	iovec_t server_iov[niov];
	for (int i = 0; i < niov; ++i) {
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
		strerror_exit(smc.smc_errno, "sendmsg");
	if (received < 0)
		strerror_exit(rmc.rmc_errno, "recvmsg_or_1_msg_in_recvmmsg");
	ensure(tn, received == sizeof(client_data));
	ensure(tn, mem_is_equal(client_data, server_data, received));
	return 0;
}

 //  Various boundary tests related to use of more than one iovec,
 //  peek at the data first, then receive it

int con_receive_iovec_boundary_checks_peeking(con_t *conp, int tn, size_t niov)
{
	size_t iov_max = iovec_max();

	int client_data[niov];
	iovec_t client_iov[niov];
	for (int i = 0; i < niov; ++i) {
		client_data[i] = i;
		iovec_init(&client_iov[i], &client_data[i],
			   sizeof(client_data[0]));
	}

	int server_data[niov];
	iovec_t server_iov[niov];
	for (int i = 0; i < niov; ++i) {
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
			strerror_exit(smc.smc_errno, "sendmsg");
		if (received < 0)
			perror_exit("recvmsg");
		ensure(tn, received == sizeof(client_data));
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
		strerror_exit(smc.smc_errno, "sendmsg");
	if (received < 0)
		perror_exit("recvmsg");
	ensure(tn, received == sizeof(client_data));
	ensure(tn, mem_is_equal(client_data, server_data, received));
	return TEST_HAS_MULTIPLE_RECVMSG;
}

 //  Test that file descriptor is received over a unix domain socket.
 //  Variations to test the close on exec flag, whether some amount of
 //  data is to be sent as regular data, and missing bytes of space for
 //  the control message to receive the file descriptor.  The missing
 //  bytes are expressed as a (signed) ssize_t controllen_delta.  Passing
 //  in cmsghdr_t requires a negative delta to be small enough so as to
 //  actually cause trancation of the space for an int sized file descriptor,
 //  a negative controllen_delta must be <= -(ssize_t)(sizeof(size_t)).

int con_receive_file_descriptor(con_t *conp, int tn, int cloexec_flag,
				bool some_data, ssize_t controllen_delta)
{
	assert(controllen_delta >= 0 ||
	       controllen_delta <= -(ssize_t) (sizeof(size_t)));
	assert(cloexec_flag == 0 || cloexec_flag == MSG_CMSG_CLOEXEC);

	int pipefd[2];
	if (pipe(pipefd) < 0)	// something to send that's easy to check
		perror_exit("pipe");
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
		strerror_exit(rmc.rmc_errno, "recvmsg_or_1_msg_in_recvmmsg");

	ensure(tn, received == expected);
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

		size_t nwrote = write(received_fd, "p", 1);
		if (nwrote < 0)
			perror_exit("write");
		ensure(tn, nwrote == 1);

		char c;
		size_t nread = read(pipe_read, &c, 1);
		if (nread < 0)
			perror_exit("read");
		ensure(tn, nread == 1);
		ensure(tn, c == 'p');

		int fdflags = fcntl(received_fd, F_GETFD, 0);
		if (fdflags == -1)
			perror_exit("fcntl");
		if (cloexec_flag)
			ensure(tn, fdflags & FD_CLOEXEC);
		else
			ensure(tn, ! (fdflags & FD_CLOEXEC));
		close(received_fd);
	}

	close(pipe_read);
	close(pipe_write);
	if (cloexec_flag)
		return TEST_CLOEXEC_FLAG;	// can not mix flags
	return 0;
}

 //  To be able to test this specification in the recvmsg(2) man page:
 //
 //	"If a message is too long to fit in the supplied buffer,
 //	 excess bytes may be discarded depending on the type of
 //	 socket the message is received from."
 //
 //  The udp(7) protocol discards the excess bytes, the sctp(7) does not.
 //  This test is meant to be used with both, tests for the excess bytes being
 //  discarded or for MSG_EOR being set together with the non-discarded bytes.
 //  The test sends a 16 byte message, but receives it with an 8 byte buffer,
 //  a second recvmsg(2) is done to see if the next 8 bytes are of a 2nd
 //  identical 16 byte packet sent or if they belong to the first packet.

int con_message_too_long_to_fit_might_discard_bytes(con_t *conp, int tn,
						    int type)
{
	assert(type == SOCK_SEQPACKET || type == SOCK_DGRAM);

	size_t total = 16;
	assert(total % 2 == 0);				// must be even
	size_t half = total / 2;

	iovec_t client_iov;
	iovec_init(&client_iov, "0123456789ABCDEF", total);
	assert(strlen(client_iov.iov_base) == total);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[half + 1];	// client sends total, receive half
	memset(&server_data, 0, half);	// 1 byte of space for GUARD_CHAR
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
		perror_exit("recvmsg");

	ensure(tn, received == half);
	ensure(tn, server_data[half] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, half));
	if (type == SOCK_SEQPACKET)
		ensure(tn, type == SOCK_SEQPACKET &&
		       server_recv.msg_flags == 0);
	else
		ensure(tn, type == SOCK_DGRAM &&
		       server_recv.msg_flags & MSG_TRUNC);

	 //  send the same data in a second from client, should receive
	 //  the same data, not the second half of the first message, if
	 //  this test passes, then the excess bytes are being discarded,
	 //  but if it receives the second half, then they are not being
	 //  discarded and MSG_EOR indicates the end of the first record

	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	thread_call(&smc.smc_thread_call, (thread_start_t) sendmsg_call);
	received = recvmsg(conp->con_server, &server_recv, MSG_TRUNC);
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		perror_exit("recvmsg");

	 //  MSG_TRUNC in this recvmsg() asks for real size of the datagram,
	 //  not the read size, for SOCK_DGRAM this should be total not half
	 //  the GUARD_CHAR check below ensures that no more than half were
	 //  read, and the check on the read bytes ensures half were returned
	 //
	 //     "MSG_TRUNC"
	 //	"For raw (AF_PACKET), Internet datagram, netlink, and UNIX
	 //	 datagram sockets: return the real length of the packet or
	 //	 datagram, even when it was longer than the passed buffer."
	 //							-- recvmsg(2)

	if (type == SOCK_DGRAM)
		ensure(tn, received == total);
	else
		ensure(tn, received == half);
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

 //  recvmsg(2):
 //
 //	"If no messages are available at the socket, the
 //	 receive calls wait for a message to arrive"
 //
 //  The sending thread waits a second prior to sending to see if the
 //  recvmsg(2) indeed waits for the message to arrive.

int con_receive_waits_for_message(con_t *conp, int tn)
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
	memset(&server_data, 0, total);	// 1 byte of space for GUARD_CHAR
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
		strerror_exit(rmc.rmc_errno, "recvmsg_or_1_msg_in_recvmmsg");

	ensure(tn, received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	return TEST_NEEDS_SLEEP_IN_SENDER;
}

 //  recvmsg(2):
 //
 //	"If no messages are available at the socket, the
 //	 receive calls wait for a message to arrive, unless 
 //	 the socket is nonblocking (see fcntl(2)), in which
 //	 case the value -1 is returned and the external variable 
 //	 errno is set to EAGAIN or EWOULDBLOCK."
 //
 //  If a message is never sent, the receiver should not block.

int con_receiver_doesnt_wait_for_messages(con_t *conp, int tn)
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
	memset(&server_data, 0, total);	// 1 byte of space for GUARD_CHAR
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
	received = recvmsg(conp->con_server, &server_recv, 0);	// now wait
	thread_join(&smc.smc_thread_call);

	if (received < 0)
		perror_exit("recvmsg");

	ensure(tn, received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	return TEST_HAS_MULTIPLE_RECVMSG;
}

 //  recvmsg(2):
 //
 //	"The receive calls normally return any data available,
 //	 up to the requested amount, rather than waiting for
 //	 receipt of the full amount requested."

int con_receive_returns_what_is_available(con_t *conp, int tn)
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
	char server_data[twice + 1];	// client sends total, want twice
	memset(&server_data, 0, twice);	// 1 byte of space for GUARD_CHAR
	server_data[total] = GUARD_CHAR;
	server_data[twice] = GUARD_CHAR;	// set two guards
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
		strerror_exit(rmc.rmc_errno, "recvmsg_or_1_msg_in_recvmmsg");

	ensure(tn, received == total);
	ensure(tn, server_data[total] == GUARD_CHAR);
	ensure(tn, server_data[twice] == GUARD_CHAR);
	ensure(tn, mem_is_equal(server_data, client_iov.iov_base, total));
	ensure(tn, server_recv.msg_flags & MSG_EOR);
	return 0;
}

 //  recvmsg(2):
 //
 //	"Datagram sockets in various domains (e.g., the UNIX and Internet
 //	 domains) permit zero-length datagrams. When such a datagram is
 //	 received, the return value is 0."

int con_empty_datagram_received_as_empty_datagram(con_t *conp, int tn)
{
	iovec_t client_iov;
	iovec_init(&client_iov, "", 0);

	msghdr_t client_send;
	msghdr_init(&client_send, &conp->con_server_sin, &client_iov, 1, 0);

	sendmsg_call_t smc;
	sendmsg_call_init(&smc, conp->con_client, &client_send, 0);

	char server_data[1];		// client sends nothing
	server_data[0] = GUARD_CHAR;	// set guard
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
		strerror_exit(rmc.rmc_errno, "recvmsg_or_1_msg_in_recvmmsg");

	ensure(tn, received == 0);
	ensure(tn, server_data[0] == GUARD_CHAR);
	return TEST_NEEDS_SLEEP_IN_SENDER;
}

 //}
 //{	The test functions above have variations based on their arguments.
 //
 //	The variations are wrapped with specific argument values into objects
 //	that derive from call_t, the wrapped objects don't change so they
 //	can be used to repeat a test multiple times for a multi-message call.
 //

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

void tests_wrap(void)
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

	 //  ctlmsgfd_t (with 64 bit size_t) gets an extra int of padding
	 //  missing bytes then has to be sizeof(size_t) not sizeof(int),
	 //  otherwise there is still space for a sizeof(int) sized fd

	call_receive_file_descriptor_init(&rfd_4, 0, some_data,
					  -sizeof(size_t));
	call_receive_file_descriptor_init(&rfd_5, 0, no_data,
					  -sizeof(size_t));
	call_receive_file_descriptor_init(&rfd_6, MSG_CMSG_CLOEXEC, some_data,
					  -sizeof(size_t));
	call_receive_file_descriptor_init(&rfd_7, MSG_CMSG_CLOEXEC, no_data,
					  -sizeof(size_t));

	 //  missing actual fields in the underlying cmsghdr_t

	call_receive_file_descriptor_init(&rfd_8, 0, some_data,
					  -(ssize_t) (2 * sizeof(size_t)));
	call_receive_file_descriptor_init(&rfd_9, 0, no_data,
					  -(ssize_t) (2 * sizeof(size_t)));
	call_receive_file_descriptor_init(&rfd_10, MSG_CMSG_CLOEXEC, some_data,
					  -(ssize_t) (2 * sizeof(size_t)));
	call_receive_file_descriptor_init(&rfd_11, MSG_CMSG_CLOEXEC, no_data,
					  -(ssize_t) (2 * sizeof(size_t)));

	 //  just one byte in the underlying cmsghdr_t

	call_receive_file_descriptor_init(&rfd_12, 0, some_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
	call_receive_file_descriptor_init(&rfd_13, 0, no_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
	call_receive_file_descriptor_init(&rfd_14, MSG_CMSG_CLOEXEC, some_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
	call_receive_file_descriptor_init(&rfd_15, MSG_CMSG_CLOEXEC, no_data,
					  -(ssize_t) (sizeof(ctlmsgfd_t) - 1));
}

 //}
 //{	Test selection and combinations of them into multi-message tests.
 //

size_t tests_select(callconinit_t cci, call_t *tests_src[],
			 call_t *tests_dest[], size_t n, int error)
{
	size_t d = 0;
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
enum { N_REPEAT = 4 };

void tests_run(void)
{
	int error;
	size_t iov_max = iovec_max();
	size_t max_tests = iov_max;
	sendmsg_call_t *smc_vec[max_tests];
	recvmsg_call_t *rmc_vec[max_tests];

	 //  Run all the tests individually.

	if (test_verbose > 1)
		printf("Running %d individual sendmsg(2) tests:\n", N_TESTS);
	call_t *callp;
	for (int i = 0; i < N_TESTS; ++i) {
		callp = call_tests[i];
		con_t con;
		callp->call_con_init(callp, &con);
		callp->call_errno = call_go(callp, &con);
		con_deinit(&con);
	}

	 //  Select all the socketpair() based test cases a test array so
	 //  they can be used together in single recvmmsg() multi-message test
	 //  that uses the same connection.

	call_t *call_tests_selected[max_tests + 1];
	assert(iov_max > N_TESTS);
	size_t n_selected = tests_select(call_do_con_init_socketpair,
					 call_tests, call_tests_selected,
					 N_TESTS, 0);

	 //  First call them individually sharing the connection one after
	 //  the other to ensure there are no dependencies, for example left-
	 //  over data leakage left in the socket between individual tests
	 //  that might break the tests

	if (test_verbose > 1)
		printf("\nRunning %d non-error socketpair() "
		       "tests sharing a connection:\n", n_selected);
	con_t con;
	con_init_socketpair(&con);
	for (int i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int error = call_go(callp, &con);
		assert(!error);
	}
	con_deinit(&con);

	 //  We have n_selected tests that can work over the same type of
	 //  connection and don't have any failures. Use them for mult-receive
	 //  recvmmsg(2) testing.
	 //
	 //  Calling the first test causes it to call in the middle of it
	 //  (unknown to it) the next one in a nested recursive call, the
	 //  same occurs for each one of them, when there are no more to
	 //  recurse all the sendmsg_call_t and recvmsg_call_t have been
	 //  gathered and they can be used to craft the single recvmmsg(2)
	 //  call which is tested both with n_selected sendmsg(2) calls or
	 //  a single call to sendmmsg(2).
	 //
	 //  The recursion occurs, unknown by the tests, in their call to:
	 //	con_add_send_recv_calls()

	for (int sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		con_init_socketpair(&con);
		if (test_verbose > 1)
			printf("\nRunning %d non-error socketpair() tests "
			       "with a single recvmmsg(2) call and %s:\n",
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

	 //  Run the recvmmsg(2) test from above with sleeps in the
	 //  sender and MSG_WAITFORONE in the receiver, the receiver
	 //  keeps on on re-trying until they are all received.

	if (test_verbose > 1)
		printf("\nRunning %d non-error socketpair() tests with as "
		       "many MSG_WAITFORONE recvmmsg(2) calls as needed and "
		       "individual sleeping sendmsg(2) calls\n", n_selected);
	con_init_socketpair(&con);
	con_make_vectored(&con, false, n_selected,
			  call_tests_selected, smc_vec, rmc_vec);
	con_make_waitforone_tests(&con);
	callp = call_tests_selected[0];
	error = call_go(callp, &con);
	assert(!error);					
	con_deinit(&con);

	 //  Run the recvmmsg(2) test from above with sleeps in the
	 //  sender and timeouts in the receiver, the receiver keeps
	 //  on on re-trying until they are all received.

	if (test_verbose > 1)
		printf("\nRunning %d non-error socketpair() tests with as "
		       "many recvmmsg(2) calls with a timeout as needed and "
		       "individual sleeping sendmsg(2) calls\n", n_selected);
	con_init_socketpair(&con);
	con_make_vectored(&con, false, n_selected,
			  call_tests_selected, smc_vec, rmc_vec);
	con_make_timeout_tests(&con);
	callp = call_tests_selected[0];
	error = call_go(callp, &con);
	assert(!error);					
	con_deinit(&con);

	 //  Get the socketpair tests that returned TEST_CLOEXEC_FLAG
	 //  and run them sequentially to ensure there are no issues with
	 //  the connection sharing.

	n_selected = tests_select(call_do_con_init_socketpair, call_tests,
				  call_tests_selected,
				  N_TESTS, TEST_CLOEXEC_FLAG);
	if (test_verbose > 1)
		printf("\nRunning %d TEST_CLOEXEC_FLAG socketpair() "
		       "tests sharing a connection:\n",
		       n_selected);
	con_init_socketpair(&con);
	for (int i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int error = call_go(callp, &con);
		assert(error == TEST_CLOEXEC_FLAG);
	}
	con_deinit(&con);

	 //  Now run them with recvmmsg(2)

	for (int sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			printf("\nRunning %d TEST_CLOEXEC_FLAG socketpair() "
			       "tests with a single recvmmsg(2) call "
			       "and %s:\n", n_selected,
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

#ifdef RECVMMSG_USE_SCTP //{
	 //  Select the SOCK_SEQPACKET tests, run them sequentially sharing
	 //  the connection to ensure there are no issues sharing it.

	n_selected = tests_select(call_do_con_init_seqpacket, call_tests,
				  call_tests_selected, N_TESTS, 0);
	if (test_verbose > 1)
		printf("\nRunning %d non-error SOCK_SEQPACKET "
		       "tests sharing a connection:\n", n_selected);
	con_init_seqpacket(&con);
	for (int i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int error = call_go(callp, &con);
		assert(!error);
	}
	con_deinit(&con);

	 //  Run the SOCK_SEQPACKET tests with multiple sendmsg(2) calls
	 //  (or a single sendmmsg(2) call) and recvmmsg(2).

	for (int sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			printf("\nRunning %d non-error SOCK_SEQPACKET tests "
			       "with a single recvmmsg(2) call and %s:\n",
			       n_selected,
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

	 //  Get the SOCK_SEQPACKET tests again, repeat them N_REPEAT times
	 //  and run them sequentially to ensure there are no issues with the
	 //  connection sharing.

	n_selected = tests_select(call_do_con_init_seqpacket, call_tests,
				  call_tests_selected, N_TESTS, 0);
	assert(N_REPEAT * n_selected < N_TESTS);
	for (int i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		for (int skip = n_selected, r = 0; r < N_REPEAT;
		     ++r, skip += n_selected)
			call_tests_selected[i + skip] = callp;
	}

	n_selected *= N_REPEAT;
	if (test_verbose > 1)
		printf("\nRunning %d non-error SOCK_SEQPACKET "
		       "tests sharing a connection (N_REPEAT = %d):\n",
		       n_selected, N_REPEAT);
	con_init_seqpacket(&con);
	for (int i = 0; i < n_selected; ++i) {
		callp = call_tests_selected[i];
		int error = call_go(callp, &con);
		assert(!error);
	}
	con_deinit(&con);

	 //  Now run them with recvmmsg(2)

	for (int sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			printf("\nRunning %d non-error SOCK_SEQPACKET tests "
			       "with a single recvmmsg(2) call (N_REPEAT = %d) "
			       "and %s:\n",
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

	 //  Repeat the first test iov_max with recvmmsg(2)

	n_selected = iov_max;
	callp = call_tests_selected[0];
	for (int i = 1; i < n_selected; ++i)
		call_tests_selected[i] = callp;

	for (int sendmulti = 0; sendmulti <= 1; ++sendmulti) {
		if (test_verbose > 1)
			printf("\nRunning %d non-error SOCK_SEQPACKET tests "
			       "with a single recvmmsg(2) call and %s:\n",
			       n_selected,
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
#endif //}
}

 //}
 //{	Wrap and run the tests.
 //

int main(int argc, char *argv[])
{
	tst_parse_opt(argc, argv);
	tests_wrap();
	tests_run();
	exit(ensure_failures > 0 ? 1 : 0);
}

 //}
 //{	Utility functions and mechanism for test combinations and for
 //	turning individual sendmsg(2) / recvmsg(2) based tests into
 //	multi-message sendmmsg(2) / recvmmsg(2) calls are in this code.
 //
 //	Test functions that use a single sendmsg() and a single recvmsg() use:
 //		con_add_send_recv_calls()
 //
 //	To either run the test with an individual sendmsg() and recvmsg(),
 //	or to have those combined with the send and receive of unrelated
 //	tests into a multi-message test.
 //
 //	The combination happens in:
 //		con_do_multi_send_recv()

void con_init_base(con_t *conp, int client, int server,
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

void con_init(con_t *conp, int client, int server,
	      int type, sockaddr_in_t *sinp)
{
	con_init_base(conp, client, server, type, sinp);
	conp->con_call_sendmmsg = false;
	conp->con_sender_sleeps = false;	// determined later
	conp->con_waitforone = false;
	conp->con_timeout = false;
	conp->con_call_end = 0;
	conp->con_call_n = 0;
	conp->con_call_vec = NULL;
	conp->con_smc_vec = NULL;
	conp->con_rmc_vec = NULL;
}

void con_make_vectored(con_t *conp, bool sendmulti,
		       size_t n, call_t **call_vec,
		       sendmsg_call_t **smc_vec, recvmsg_call_t **rmc_vec)
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

void con_make_waitforone_tests(con_t *conp)
{
	assert(conp->con_call_vec);
	assert(!conp->con_call_sendmmsg);
	conp->con_timeout = true;
}

void con_make_timeout_tests(con_t *conp)
{
	assert(conp->con_call_vec);
	assert(!conp->con_call_sendmmsg);
	conp->con_waitforone = true;
}

void con_add_send_recv_calls_vec(con_t *conp, sendmsg_call_t *smcp,
				 recvmsg_call_t *rmcp, bool sender_sleeps)
{
	assert(conp->con_call_end < conp->con_call_n);
	int i = conp->con_call_end;

	 //  All senders must either: need to sleep or must not sleep.

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

void *sendmmsg_call(sendmmsg_call_t *smmcp)
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

void con_sendmmsg_recvmmsg(con_t *conp, size_t n,
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
		assert(nrecv == n);
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
	assert(nsent == n);
}

void con_do_multi_send_recv(con_t *conp)
{
	assert(conp->con_call_n >= 1 && conp->con_call_n <= iovec_max());
	size_t n = conp->con_call_n;
	mmsghdr_t smm[n];
	mmsghdr_t rmm[n];

	 //  Gather the arguments from all the sendmsg_call_t and the
	 //  recvmsg_call_t into the smm[] and rmm[] vectors for the
	 //  multi-message send and receive syscalls.

	 //  All the send and recv flags must be the same, there is
	 //  a single argument for the flags in sendmmsg(2) and in
	 //  recvmmsg(2), there is not an argument per message.

	int sflags = conp->con_smc_vec[0]->smc_flags;
	int rflags = conp->con_rmc_vec[0]->rmc_flags;
	for (int i = 0; i < n; i++) {
		assert(conp->con_smc_vec[i]->smc_flags == sflags);
		assert(conp->con_rmc_vec[i]->rmc_flags == rflags);
		mmsghdr_init(&smm[i], conp->con_smc_vec[i]->smc_msg);
		mmsghdr_init(&rmm[i], conp->con_rmc_vec[i]->rmc_msg);
	}

	con_sendmmsg_recvmmsg(conp, n, sflags, smm, rflags, rmm);

	 //  Scatter the results from smm[] and rmm[].

	for (int i = 0; i < n; i++) {
		conp->con_smc_vec[i]->smc_value = smm[i].msg_len;
		conp->con_smc_vec[i]->smc_msg->msg_flags =
						  smm[i].msg_hdr.msg_flags;
		conp->con_rmc_vec[i]->rmc_value = rmm[i].msg_len;
		conp->con_rmc_vec[i]->rmc_msg->msg_flags =
						  rmm[i].msg_hdr.msg_flags;
	}
}

void con_init_socketpair(con_t *conp)
{
	int pair[2];
	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pair) < 0)
		perror_exit("socketpair");
	con_init(conp, pair[0], pair[1], SOCK_SEQPACKET, NULL);
}

void con_init_seqpacket(con_t *conp)
{
	con_init_with_type(conp, SOCK_SEQPACKET);
}

void con_init_dgram(con_t *conp)
{
	con_init_with_type(conp, SOCK_DGRAM);
}

void con_init_with_type(con_t *conp, int type)
{
#ifndef RECVMMSG_USE_SCTP
	assert(type != SOCK_SEQPACKET);
#endif
	int protocol = (type == SOCK_SEQPACKET) ? IPPROTO_SCTP : 0;
	int server = socket(PF_INET, type, protocol);
	if (server < 0)
		perror_exit("socket");

	sockaddr_in_t sin;
	sockaddr_in_init(&sin, AF_INET, TEST_PORT, TEST_IP);
	if (bind(server, (sockaddr_t *) &sin, sizeof(sin)) < 0)
		perror_exit("bind");

	int client = socket(PF_INET, type, protocol);
	if (client < 0)
		perror_exit("socket");

	if (type == SOCK_SEQPACKET && listen(server, 1) < 0)
		perror_exit("listen");

	con_init(conp, client, server, type, &sin);
}

void con_deinit(con_t *conp)
{
	close(conp->con_client);
	close(conp->con_server);
}

void con_add_send_recv_calls(con_t *conp, sendmsg_call_t *smcp,
			     recvmsg_call_t *rmcp, bool sender_sleeps)
{
	if (conp->con_call_n > 0) {
		con_add_send_recv_calls_vec(conp, smcp, rmcp, sender_sleeps);
		return;
	}

	thread_call(&smcp->smc_thread_call, (thread_start_t)
		    (sender_sleeps ? sendmsg_call_after_sleep : sendmsg_call));
	timeval_t pretv, postv;
	if (sender_sleeps && gettimeofday(&pretv, NULL) < 0)
		perror_exit("gettimeofday()");
	ssize_t val = recvmsg(conp->con_server, rmcp->rmc_msg, rmcp->rmc_flags);
	if (val < 0)
		rmcp->rmc_errno = errno;
	if (sender_sleeps) {

		 //  If the sender is supposed to sleep prior to sending then
		 //  the receiver is supposed to sleep too because its receive
		 //  should block for the sender, testing that sender blocked
		 //  for at least half as long as the sender sleeps is good
		 //  enough to ensure that scheduling noise doesn't affect test.

		if (gettimeofday(&postv, NULL) < 0)
			perror_exit("gettimeofday()");
		unsigned long long pre = pretv.tv_sec * 1000;
		pre += pretv.tv_usec * 1000;
		unsigned long long pos = postv.tv_sec * 1000;
		pos += postv.tv_usec * 1000;
		assert((pos - pre) > SENDMSG_SLEEP_MS / 2);
	}

	rmcp->rmc_value = val;
	thread_join(&smcp->smc_thread_call);
}

 //}
 //{	Initialization of test functions into wrapped tests into classes
 //	that dervice from call_t. For each test function there is an init
 //	function and a test running function. All the test running funcitons
 //	have the same signature, so the calling mechainsm doesn't have to
 //	know about their arguments which are wrapped into the classes that
 //	inherit from call_t.
 //

 //  Tests bundled into their own call_t types so they can be intermixed
 //  in multi-message sends and receives (sendmmsg(2) and recvmmsg(2)

void call_receive_iovec_boundary_checks_init(
	call_receive_iovec_boundary_checks_t *callp, size_t niov)
{
	call_base_init(&callp->call_base, call_do_con_init_socketpair,
		       (calltest_t) call_receive_iovec_boundary_checks);
	callp->call_niov = niov;
}
int call_receive_iovec_boundary_checks(
	call_receive_iovec_boundary_checks_t *callp, con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_receive_iovec_boundary_checks(niov = %d)"
		       " // iov_max = %d\n",
		       (int) callp->call_niov, (int) iovec_max());
	return con_receive_iovec_boundary_checks(conp, tn, callp->call_niov);
}

void call_receive_iovec_boundary_checks_peeking_init(
	call_receive_iovec_boundary_checks_peeking_t *callp, size_t niov)
{
	call_base_init(&callp->call_base, call_do_con_init_socketpair,
		       (calltest_t) call_receive_iovec_boundary_checks_peeking);
	callp->call_niov = niov;
}
int call_receive_iovec_boundary_checks_peeking(
	call_receive_iovec_boundary_checks_peeking_t *callp,
	con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_receive_iovec_boundary_checks_peeking(niov = %d)"
		       " // iov_max = %d\n",
		       (int) callp->call_niov, (int) iovec_max());
	return con_receive_iovec_boundary_checks_peeking(conp, tn,
							 callp->call_niov);
}

void call_receive_file_descriptor_init(
	call_receive_file_descriptor_t *callp,
	int cloexec_flag, bool some_data, ssize_t controllen_delta)
{
	call_base_init(&callp->call_base, call_do_con_init_socketpair,
		       (calltest_t) call_receive_file_descriptor);
	callp->call_cloexec_flag = cloexec_flag;
	callp->call_some_data = some_data;
	callp->call_controllen_delta = controllen_delta;
}
int call_receive_file_descriptor(
	call_receive_file_descriptor_t *callp, con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_receive_file_descriptor("
		       "cloexec_flag = %s, some_data = %d, "
		       "controllen_delta = %ld)\n",
		       (callp->call_cloexec_flag & MSG_CMSG_CLOEXEC) ?
					"MSG_CMSG_CLOEXEC" : "0",
		       callp->call_some_data,
		       (long) callp->call_controllen_delta);
	return con_receive_file_descriptor(conp, tn, callp->call_cloexec_flag,
					   callp->call_some_data,
					   callp->call_controllen_delta);
}

void call_msgtoolong_do_con_init_from_type(
	call_message_too_long_to_fit_might_discard_bytes_t *callp, con_t *conp);

void call_msgtoolong_do_con_init_from_type(
	call_message_too_long_to_fit_might_discard_bytes_t *callp, con_t *conp)
{
	con_init_with_type(conp, callp->call_type);
}

void call_message_too_long_to_fit_might_discard_bytes_init(
	call_message_too_long_to_fit_might_discard_bytes_t *callp,
	int type)
{
	call_base_init(&callp->call_base,
		       (callconinit_t) call_msgtoolong_do_con_init_from_type,
		       (calltest_t)
		       call_message_too_long_to_fit_might_discard_bytes);
	callp->call_type = type;
}
int call_message_too_long_to_fit_might_discard_bytes(
	call_message_too_long_to_fit_might_discard_bytes_t *callp,
	con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_message_too_long_to_fit_might_discard_bytes("
		       "type = %s)\n",
		       sock_type_to_str(callp->call_type));
	return con_message_too_long_to_fit_might_discard_bytes(conp, tn,
							callp->call_type);
}

void call_empty_datagram_received_as_empty_datagram_init(
	call_empty_datagram_received_as_empty_datagram_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_dgram,
		       (calltest_t)
		       call_empty_datagram_received_as_empty_datagram);
}
int call_empty_datagram_received_as_empty_datagram(
	call_empty_datagram_received_as_empty_datagram_t *callp,
	con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_empty_datagram_received_as_empty_datagram()\n");
	return con_empty_datagram_received_as_empty_datagram(conp, tn);
}

#ifdef RECVMMSG_USE_SCTP //{

void call_receive_waits_for_message_init(
	call_receive_waits_for_message_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_seqpacket,
		       (calltest_t) call_receive_waits_for_message);
}
int call_receive_waits_for_message(
	call_receive_waits_for_message_t *callp, con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_receive_waits_for_message()\n");
	return con_receive_waits_for_message(conp, tn);
}

void call_receiver_doesnt_wait_for_messages_init(
	call_receiver_doesnt_wait_for_messages_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_seqpacket,
		       (calltest_t) call_receiver_doesnt_wait_for_messages);
}
int call_receiver_doesnt_wait_for_messages(
	call_receiver_doesnt_wait_for_messages_t *callp, con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_receiver_doesnt_wait_for_messages()\n");
	return con_receiver_doesnt_wait_for_messages(conp, tn);
}

void call_receive_returns_what_is_available_init(
	call_receive_returns_what_is_available_t *callp)
{
	call_base_init(&callp->call_base, call_do_con_init_seqpacket,
		       (calltest_t) call_receive_returns_what_is_available);
}
int call_receive_returns_what_is_available(
	call_receive_returns_what_is_available_t *callp, con_t *conp, int tn)
{
	print_nest(tn);
	if (test_verbose)
		printf("con_receive_returns_what_is_available()\n");
	return con_receive_returns_what_is_available(conp, tn);
}

#endif //}

 //}
 //{	Miscellaneous supporting code is in this section.
 //

void perror_exit_file_line(const char *s, const char *file,
			   const char *func, int line)
{
	fprintf(stderr, "%s: %s:%d %s() ", prog_name, file, line, func);
	perror(s);
	exit(1);
}

void strerror_exit_file_line(int e, const char *s,
			     const char *file, const char *func, int line)
{
	fprintf(stderr, "%s: %s:%d %s() %s: %s\n",
		prog_name, file, line, func, s, strerror(e));
	exit(1);
}

void tst_parse_opt(int argc, char *argv[])
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

int errno_value_is_error(int e)
{
	return e > 0;	// negative values are not errors, see non_error_errno
}

void sleep_ms(long ms)
{
	assert(ms < 1000);
	timespec_t ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ms * 1000 * 1000;
	nanosleep(&ts, NULL);
}

void print_nest(int test_number)
{
	if (test_verbose) {
		if (subtest_number() == 1)
			printf("\n");
		print_prefix(test_number);
	}
}

void print_prefix(int test_number)
{
	if (test_verbose) {
		printf("%s%5d        :  [subtest=%d] ",
		       TCID, test_number, subtest_number());
	}
}

int test_number_get(void)
{
	static int test_number_gen;

	 //  Nested subtests share the same test number, these are multi
	 //  message tests, i.e. recvmmsg() and sendmmsg(), where each message
	 //  is treated as a subtest

	if (subtest_number() > 1)
		return test_number_gen;
	return ++test_number_gen;
}

 //  subtests numbers

int subtest_level = 1;

int subtest_number(void)
{
	return subtest_level;
}

void subtest_nest(void)
{
	++subtest_level;
}

void subtest_unnest(void)
{
	--subtest_level;
}

 //  compare that memory is equal, makes the output of various ensure() nicer

bool mem_is_equal(void *a, void *b, size_t size)
{
	return !memcmp(a, b, size);
}

bool mem_is_zero(void *a, size_t size)
{
	unsigned char set = 0;
	unsigned char *p = a;
	unsigned char *endp = p + size;
	while (p < endp) set |= *p++;
	return !set;
}

void thread_call(thread_call_t *tcp, thread_start_t func)
{
	int error = pthread_create(&tcp->tc_thr, NULL, func, tcp);
	if (error)
		strerror_exit(error, "pthread_create");
}

void thread_join(thread_call_t *tcp)
{
	int error = pthread_join(tcp->tc_thr, NULL);
	if (error)
		strerror_exit(error, "pthread_join");
}

void sendmmsg_init(sendmmsg_call_t *smmcp, int sockfd, unsigned vlen,
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

 //  various init and member functions for various types

void sockaddr_in_init(sockaddr_in_t *sinp, sa_family_t family,
		      in_port_t port, uint32_t ip)
{
	sockaddr_in_init_to_zero(sinp);
	sinp->sin_family = family;
	sinp->sin_port = htons(port);
	sinp->sin_addr.s_addr = htonl(ip);
}

void sockaddr_in_init_to_zero(sockaddr_in_t *sinp)
{
	memset(sinp, 0, sizeof(*sinp));
}

char *sock_type_to_str(int type)
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

size_t iovec_max(void)
{
	long iov_max = sysconf(_SC_IOV_MAX);
	if (iov_max == -1)
		perror_exit("sysconf");
	assert(iov_max > 2);
	return (size_t) iov_max;
}

void iovec_init(iovec_t *iovecp, void *base, size_t len)
{
	iovecp->iov_base = base;
        iovecp->iov_len = len;
}

void msghdr_init(msghdr_t *msgp, sockaddr_in_t *sinp,
		 iovec_t *iovecp, size_t iovlen, int flags)
{
	msghdr_init_with_control(msgp, sinp, iovecp, iovlen, NULL, 0, flags);
}

void msghdr_init_with_control(msghdr_t *msgp, sockaddr_in_t *sinp,
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

void mmsghdr_init(mmsghdr_t *mmsgp, msghdr_t *msgp)
{
	mmsgp->msg_hdr = *msgp;
	mmsgp->msg_len = 0;
}

void recvmsg_call_init(recvmsg_call_t *rmcp, msghdr_t *msgp, int flags)
{
	rmcp->rmc_value = INVALID_SYSCALL_VALUE;
	rmcp->rmc_errno = 0;
	rmcp->rmc_msg = msgp;
	rmcp->rmc_flags = flags;
}

void sendmsg_call_init(sendmsg_call_t *smcp, int sockfd,
		       msghdr_t *msgp, int flags)
{
	smcp->smc_value = INVALID_SYSCALL_VALUE;
	smcp->smc_errno = 0;
	smcp->smc_sockfd = sockfd;
	smcp->smc_msg = msgp;
	smcp->smc_flags = flags;
}

 //  functions that can be called directly through pthread_create(3)

void *sendmsg_call(sendmsg_call_t *smcp)
{
	errno = 0;
	smcp->smc_value = sendmsg(smcp->smc_sockfd, smcp->smc_msg,
				  smcp->smc_flags);
	smcp->smc_errno = errno;
	return NULL;
}

void *sendmsg_call_after_sleep(sendmsg_call_t *smcp)
{
	sleep_ms(SENDMSG_SLEEP_MS);
	return sendmsg_call(smcp);
}

void ctlmsgfd_init(ctlmsgfd_t *cmfp, int fd)
{
	memset(cmfp, 0, sizeof(*cmfp));
	cmfp->cmf_cmsg.cmsg_len = sizeof(*cmfp);
        cmfp->cmf_cmsg.cmsg_level = SOL_SOCKET;
        cmfp->cmf_cmsg.cmsg_type = SCM_RIGHTS;
	*(int *) CMSG_DATA(&cmfp->cmf_cmsg) = fd;
}

void ctlmsgfd_init_to_zero(ctlmsgfd_t *cmfp)
{
	memset(cmfp, 0, sizeof(*cmfp));
}

int ctlmsgfd_get_fd(ctlmsgfd_t *cmfp)
{
	return *(int *) CMSG_DATA(&cmfp->cmf_cmsg);
}

int call_go(call_t *callp, con_t *conp)
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
		if (test_verbose > 0)
			printf("%s%5d  %s  :  [%d subtests]\n", TCID, tn,
			       pass ? "PASS" : "FAIL", max_subtest_level);
		else
			tst_res(pass ? TPASS : TFAIL, "[%d subtests]",
				max_subtest_level);
		max_subtest_level = 1;
	}
	return error;
}

void call_do_con_init_socketpair(call_t *callp, con_t *conp)
{
	con_init_socketpair(conp);
}
void call_do_con_init_seqpacket(call_t *callp, con_t *conp)
{
	con_init_seqpacket(conp);
}
void call_do_con_init_dgram(call_t *callp, con_t *conp)
{
	con_init_dgram(conp);
}

void call_base_init(call_t *callp, callconinit_t coninit, calltest_t test)
{
	callp->call_con_init = coninit;
	callp->call_test = test;
	callp->call_errno = 0;
}

 //}
