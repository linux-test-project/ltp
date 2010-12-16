/*
 *   Copyright (C) Bull S.A. 1996
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
/*---------------------------------------------------------------------+
|                            pipe_test_02                              |
| ==================================================================== |
|                                                                      |
| Description:  Max data transfer through pipe interprocess channel    |
|               in non-blocking mode                                   |
|                                                                      |
| Algorithm:    o  Create a pipe                                       |
|               o  Make write & read end of pipe non-blocking          |
|               o  Spawn a child process                               |
|               o  parent:                                             |
|                  -  create & send data packets to the child          |
|                  -  compute checksum on sent packets                 |
|               o  child:                                              |
|                  -  recieve packets from parent & compute checksum   |
|                  -  send final checksum to parent                    |
|               o  parent:                                             |
|                  -  compare checksum of sent packets with the        |
|                     child's checksum                                 |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               pipe () - Creates an interprocess channel              |
|               fork () - Creates a new process                        |
|               fcntl () -                                             |
|               waitpid () - Waits for a child process to stop or      |
|                                                                      |
| Usage:        pipe_test_02                                           |
|                                                                      |
| To compile:   cc -o pipe_test_02 pipe_test_02.c                      |
|                                                                      |
| Last update:   Ver. 1.3, 3/3/94 12:06:38                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     010393  DJK   Initial version for AIX 4.1                 |
|    1.2     021394  DJK   Move to "prod" directory                    |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Defines:
 *
 * MB: one megabyte (MB)
 *
 * VALID_PACKET: value sent with each packet, used to verify that the
 * packets contents were not garbled.
 *
 * DEFAULT_NUM_CHILDREN: default number of child processes spawned
 *
 * DEFAULT_PACKETS_TO_SEND: default number of packets sent to each child
 * process.
 *
 * MAXCHILD: maximum number of child processes which may be spawned
 * (based upon the number of file descriptors that a process may use)
 *
 * USAGE: usage statement
 */
#define MB			(1024*1024)
#define DEFAULT_PACKETS_TO_SEND 1024
#define DEFAULT_NUM_CHILDREN	1
#define OPEN_MAX		256
#define MAXCHILD 		(OPEN_MAX/2 - 2)
#define VALID_PACKET		0xabcdef01
#define USAGE	"\nUsage: %s [-n] [-p nprocs] [{-m totmegs | -b totbytes}]\n\n" \
		"\t-n          transfer data with NON-BLOCKING reads & writes\n" \
		"\t-p nprocs   number of child processes to spawn\n" \
		"\t-m totmegs  number of MB to send through pipe\n" \
		"\t-b totmegs  number of bytes to send through pipe\n" \
		"\t                  (must be less than %d)\n\n"

/*
 * Function Prototypes:
 *
 * setup (): Parse command line arguments and intialize variables
 * child (): Child process
 * cleanup (): Close all pipes and kill child processes
 * sys_error (): System error message function
 * error (): Error message function
 * setup_signal_handlers (): Sets up signal catching functions
 * handler (): Signal catching function
 */
void setup (int, char **);
void child (int [], int []);
void cleanup ();
void sys_error (const char *, int);
void error (const char *, int);
void setup_signal_handlers ();
void handler (int, int, struct sigcontext *);

/*
 * Structures & Global variables
 *
 * num_children: number of child processes to be spawned
 *
 * num_packets: number of packets to be sent to each child process
 *
 * non_blocking_flag: uses NON-BLOCKING
 *
 * pid: process id's of the spawned processes
 *
 * p2child: half duplex pipes from parent to child (parent writes,
 *          child reads).
 *
 * p2parent: half duplex pipe from child to parent (child writes,
 *           parent reads).
 */

enum { READ, WRITE };		/* Pipe read & write end indices */

struct data_packet {
	pid_t		pid;		/* Child process id */
	int 		last;		/* Indicates last packet when set */
	long 		valid;		/* Insure packet was not garbled */
	long 		seq_number;	/* Packet sequence number */
	unsigned long	checksum;	/* Cumulative checksum so far */
	unsigned char	data;		/* Data sent in packet */
};
typedef struct data_packet data_packet;

int     num_children = DEFAULT_NUM_CHILDREN;
long	num_packets  = DEFAULT_PACKETS_TO_SEND;
int	non_blocking_flag = 0;	/* Uses NON-BLOCKING pipes when set */
int	bflg = 0;		/* Data quantity flag (MB) */
int	mflg = 0;		/* Data quantity flag (bytes) */

pid_t	parent_pid;		/* Parent's process id */
pid_t	pid [MAXCHILD];		/* Process id's of spawned processes */
int	p2child [MAXCHILD][2]; 	/* Pipes from parent to child processes */
int	p2parent [2];  		/* Pipe from child processes to parent */
char	err_msg [256];		/* Generic error message buffer */

/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
| Returns:   (0)  Successful completion                                |
|            (-1) Error occurred                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	int 	i;
	int 	n;		/* Number of bytes written */
	int	status;		/* Child's exit status */
	long	packets_sent;
	unsigned char 	data;
	unsigned long 	cksum_parent = 0;
	data_packet	packet;

	/*
	 * Parse command line arguments, initialize global variables and
	 * print program header
	 */
	setup (argc, argv);
	printf ("%s: IPC Pipe TestSuite program\n", *argv);
	fflush (stdout);

	/*
	 * Create two sets of half duplex pipes:
	 *
	 * p2child: for sending packets from the parent process to the child
	 *          processes and
	 * p2parent: for sending checksums from the child processes to the
	 *           parent
	 *
	 * If the non-blocking command line option was specified, use fcntl ()
	 * to set the O_NONBLOCK file descriptor status flags.  This will
	 * prevent reads & and writes from blocking if the data is not yet
	 * available
	 */
	printf ("\n\tCreating pipes...\n");
	fflush (stdout);

	if (pipe (p2parent) < 0)
		sys_error ("pipe failed", __LINE__);

	if (non_blocking_flag) {
		printf ("\n\tSending data NON-BLOCKING!\n");
		fflush (stdout);
	}

	for (i=0; i<num_children; i++) {
		if (pipe (&p2child [i][0]) < 0)
			sys_error ("pipe failed", __LINE__);
		if (non_blocking_flag) {
			if (fcntl (p2child [i][READ], F_SETFL, O_NONBLOCK) < 0)
				sys_error ("fcntl (O_NONBLOCK) failed", __LINE__);
			if (fcntl (p2child [i][WRITE], F_SETFL, O_NONBLOCK) < 0)
				sys_error ("fcntl (O_NONBLOCK) failed", __LINE__);
		}
	}

	/*
	 * Spawn num_children processes
	 *
	 * Fork of the child process & record the newly created process's
	 * id for future reference.
	 *
	 * Then close the READ end of the p2child pipe, since the parent
	 * process will be writing into this pipe rather than reading.
	 * Also close the WRITE end of the p2parent pipe, for just the
	 * the reverse reasons...
	 */
	printf ("\n\tSpawning %d child processes ... \n", num_children);
	fflush (stdout);

	for (i=0; i<num_children; i++) {

		if ((pid [i] = fork()) == 0) {

			/* Child process */
			child (&p2child[i][0], p2parent);
			exit (0);

		} else if (pid [i] < (pid_t)0)
			sys_error ("fork failed", __LINE__);

		if (close (p2child [i][READ]) < 0)
			sys_error ("close failed", __LINE__);
	}
	if (close (p2parent [WRITE]) < 0)
		sys_error ("close failed", __LINE__);

	/*
	 * Send data packets to the child processes
	 *
	 * Build packets (initialize all of the packets fields) and then
	 * send the packets to all of the child processes.
	 *
	 * Might have to make several attempts with the NON-BLOCKING writes
	 * if the resource is not immediately available.
	 */
	printf ("\n\tParent: sending %ld packets (%ld bytes) to child processes ...\n",
		num_packets, num_packets * sizeof (struct data_packet));

	packet.last = 0;
	packet.valid = VALID_PACKET;

	for (packets_sent = data = 0; num_packets > 0; num_packets--) {

		packet.seq_number = ++packets_sent;
		packet.data = data++;
		packet.pid = pid [i];
		packet.checksum = cksum_parent += packet.data;

		for (i=0; i<num_children; i++) {
			try_write_ETXN_again:
			if ((n = write (p2child [i][WRITE], &packet,
					sizeof (packet))) < 0) {
				if (non_blocking_flag && errno == EAGAIN) {
					goto try_write_ETXN_again;
				} else {
					sys_error ("write failed", __LINE__);
				}
			}
		}
	}

	/*
	 * Send the last packet to the child processes
	 *
	 * [ Upon receiving this packet, the child process will know that all
	 *   of the packets have been sent and that the parent process is
	 *   expecting the child to send it's checksum back. ]
	 *
	 * After sending the last packet, close the WRITE end of the p2child
	 * pipe as we are finish sending packets to the child processes.
	 *
	 * Then wait for all of the child processes to send the checksum
	 * packets.  Upon receiving the checksum packets verify that the
	 * child's checksum matches that of the parent.
	 *
	 * Might have to make several attempts with the NON-BLOCKING writes
	 * if the resource is not immediately available.
	 *
	 * Finally, close READ end of p2parent pipe as we have finished
	 * receiving checksums from the child.
	 */
	packet.last = 1;
	printf ("\n\tParent: done sending packets & waiting for children to complete!\n");
	for (i=0; i<num_children; i++) {
		try_read_again:
		if (write (p2child [i][WRITE], &packet, sizeof (packet)) < 0) {
			if (non_blocking_flag && errno == EAGAIN) {
				goto try_read_again;
			} else {
				sys_error ("write failed", __LINE__);
			}
		}
		if (close (p2child [i][WRITE]) < 0)
			sys_error ("close failed", __LINE__);

		if (read (p2parent [READ], &packet, sizeof (packet)) <= 0)
			sys_error ("read failed", __LINE__);

		if (packet.valid != VALID_PACKET)
			error ("received packet with corrupted data from child!",
				__LINE__);

		if (cksum_parent != packet.checksum) {
			sprintf (err_msg, "checksum of data sent by parent " \
				"does not match checksum of data received by " \
				"child [pid %d]\n"	\
				"\tchild's checksum: %08lx\n" \
				"\tparent's checksum: %08lx\n",
				packet.pid, packet.checksum, cksum_parent);
			error (err_msg, __LINE__);
		}
	}
	if (close (p2parent [READ]) < 0)
		sys_error ("close failed", __LINE__);

	/*
	 * Wait for all of the child processes to complete & check their
	 * exit status.
	 *
	 * Upon completion of the child proccesses, exit program with success.
	 */
	for (i=0; i<num_children; i++) {
		waitpid (pid [i], &status, 0);

		if (!WIFEXITED (status))
			sys_error ("child process terminated abnormally",
				__LINE__);
	}
	printf ("\n\tParent: children received all packets & exited successfully\n");

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");

	return (0);
}

/*---------------------------------------------------------------------+
|                               child ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Receive packets from the parent, insure they are valid    |
|            and not out of sequence, and calculate a running          |
|            checksum.  Upon receiving the last packet from the        |
|            parent, build a checksum packet and send it to the parent.|
|                                                                      |
| Args:      p2child   - Pipe from parent to child                     |
|            p2parent  - Pipe from child to parent                     |
|                                                                      |
| Returns:   Exits with (-1) if an error occurs                        |
|                                                                      |
+---------------------------------------------------------------------*/
void child (int p2child [], int p2parent [])
{
	int	n;			/* Bytes read */
	pid_t	pid = getpid ();	/* Process id of child */
	int	end_of_transmission = 0;
	long	packets_received = 0;	/* Number of packets received
					 * from parent
					 */

	data_packet 	packet;		/* Packet used to transmiting data */
	unsigned long 	cksum_child = 0;/* Checksum of data fields received */

	/*
	 * Close the WRITE end of the p2child pipe, since the child
	 * process will be reading from this pipe rather than writing.
	 * Also close the READ end of the p2parent pipe, for just the
	 * the reverse reasons...
	 */
	if (close (p2child [WRITE]) < 0)
		sys_error ("close failed", __LINE__);
	if (close (p2parent [READ]) < 0)
		sys_error ("close failed", __LINE__);

	/*
	 * Receive packets from parent & insure packets are valid
	 *
	 * Read packets from the parent through p2child pipe.  Upon
	 * recieving the packet, verify that it is valid, in sequence
	 * and that both the parent's and child's checksums match.
	 *
	 * Might have to make several attempts with the NON-BLOCKING
	 * reads if the resource is not immediately available.
	 *
	 * Continue reading packets until the "last" packet is received
	 * from the parent.  Upon receiving the last packet, close
	 * the p2child READ pipe, as we are finished receiving packets
	 * from the parent.
	 */
	while (!end_of_transmission) {
		try_write_again:
		n = read (p2child [READ], &packet, sizeof (packet));
		if (n < 0) {
			/* Resource not available */
			if (non_blocking_flag && errno == EAGAIN)
				goto try_write_again;
			else
				sys_error ("read failed", __LINE__);
		} else if (n > 0) {
			/* Insure packet is valid */
			if (packet.valid != VALID_PACKET) {
				sprintf (err_msg,
					"child received invalid packet " \
					"from parent:\n\tpacket #: %ld\n",
					packets_received);
				error (err_msg, __LINE__);
			}
			/* Received last packet */
			if (packet.last) {
				end_of_transmission = 1;
			} else {

				/* Insure packet was not received out of sequence */
				packets_received++;
				if (packets_received != packet.seq_number) {
					sprintf (err_msg,
						"child received packet out of sequence\n" \
						"\texpecting packet: %ld\n" \
						"\treceived packet:  %ld\n",
						packets_received, packet.seq_number);
					error (err_msg, __LINE__);
				}

				/* Insure checksums still match */
				cksum_child += packet.data;
				if (cksum_child != packet.checksum) {
					sprintf (err_msg,
						"child & parent checksums do not match\n" \
						"\tchild checksum:  %08lx\n" \
						"\tparent checksum: %08lx\n" \
						"\tpacket number:   %ld\n",
						cksum_child, packet.checksum, packets_received);
					error (err_msg, __LINE__);
				}
			}
		}
	}
	if (close (p2child [READ]) < 0)
		sys_error ("close failed", __LINE__);

	/*
	 * Send parent packet containing child's checksum
	 *
	 * Build a checksum packet (initialize packet fields) and then
	 * send the packet to the parent.
	 *
	 * Then close the WRITE p2parent pipe as we have finished sending packets
	 * to the parent.
	 */
	printf ("\t\tChild:  pid [%d] received %ld packets from parent\n",
		pid, packets_received);

	packet.pid = pid;
	packet.valid = VALID_PACKET;
	packet.checksum = cksum_child;

	if (write (p2parent [WRITE], &packet, sizeof (packet)) < 0)
		sys_error ("write failed", __LINE__);
	if (close (p2parent [WRITE]) < 0)
		sys_error ("close failed", __LINE__);
}

/*---------------------------------------------------------------------+
|                               setup ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Parse the command line arguments & initialize global      |
|            variables.                                                |
|                                                                      |
| Updates:   (command line options)                                    |
|                                                                      |
|            [-n] non_blocking_flag: prevents read & write calls from  |
|                 from blocking if the resource is not available.      |
|                                                                      |
|            [-p] num_packets: number of packets ...                   |
|                                                                      |
|            [-c] num_children: number of child processes to spawn ... |
|                                                                      |
+---------------------------------------------------------------------*/
void setup (int argc, char **argv)
{
	int	i;
	int	errflag = 0;
	int 	bytes = 0, megabytes = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	while ((i = getopt(argc, argv, "nm:b:p:?")) != EOF) {
		switch (i) {
			case 'n':		/* NON-BLOCKING flag */
				non_blocking_flag++;
				break;
			case 'm':		/* MB */
				mflg++;
				megabytes = atoi (optarg);
				break;
			case 'b':		/* bytes */
				bflg++;
				bytes = atoi (optarg);
				break;
			case 'p':		/* number of child procs */
				num_children = atoi (optarg);
				break;
			case '?':
				errflag++;
				break;
		}
	}
	if (mflg) {
		num_packets = megabytes * MB / sizeof (struct data_packet);
	} else if (bflg) {
		num_packets = bytes / sizeof (struct data_packet);
	}

	if (num_packets == 0 || num_children == 0 || num_children > MAXCHILD)
		errflag++;

	if (errflag) {
		fprintf (stderr, USAGE, program_name, MAXCHILD);
		exit (2);
	}
	/*
	 * Setup signal catching function for SIGPIPE & SIGINT, record
	 * the process id of the parent and initialize the child process
	 * id array.
	 */
	setup_signal_handlers ();

	parent_pid = getpid ();

	for (i=0; i<num_children; i++) {
		pid [i] = (pid_t)0;
	}
}

/*---------------------------------------------------------------------+
|                          setup_handler ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Setup the signal handler for SIGPIPE.                     |
|                                                                      |
+---------------------------------------------------------------------*/
void setup_signal_handlers ()
{
	struct sigaction invec;

	invec.sa_handler = (void (*)(int)) handler;
	sigemptyset (&invec.sa_mask);
	invec.sa_flags = 0;

	if (sigaction (SIGINT, &invec, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction failed", __LINE__);

	if (sigaction (SIGPIPE, &invec, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction failed", __LINE__);
}

/*---------------------------------------------------------------------+
|                             handler ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Signal catching function for SIGPIPE signal.              |
|                                                                      |
|            o  SIGPIPE: Print message and abort program...            |
|                                                                      |
|            o  SIGINT:  Parent process calls cleanup, child processes |
|                        simply exit                                   |
|                                                                      |
|            o  Other:   Print message and abort program...            |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int sig, int code, struct sigcontext *scp)
{
	char 	msg [100];	/* Buffer for error message */

	if (sig == SIGPIPE) {
		error ("wrote to pipe with closed read end", __LINE__);
	} else if (sig == SIGINT) {
		if (getpid () == parent_pid) {

			fprintf (stderr, "Received SIGINT -- cleaning up...\n");
			fflush (stderr);

			cleanup ();
		}
		else
			exit (-1);
	} else {
		sprintf (msg, "Received an unexpected signal (%d)", sig);
		error (msg, __LINE__);
	}
}

/*---------------------------------------------------------------------+
|                             cleanup ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Closes all of the pipes, kills all of the child           |
|            processes and exits the program...                        |
|                                                                      |
+---------------------------------------------------------------------*/
void cleanup ()
{
	int i;

	if (getpid () == parent_pid) {
		for (i=0; i<num_children; i++) {
			if (pid [i] > (pid_t)0 && kill (pid [i], SIGKILL) < 0)
				sys_error ("signal failed", __LINE__);

			close (p2child [i][READ]);
			close (p2child [i][WRITE]);
			close (p2parent [READ]);
			close (p2parent [WRITE]);
		}
	}

	exit (-1);
}

/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
void sys_error (const char *msg, int line)
{
	char syserr_msg [256];

	sprintf (syserr_msg, "%s: %s\n", msg, strerror (errno));
	error (syserr_msg, line);
}

/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and calls cleanup...                   |
|                                                                      |
+---------------------------------------------------------------------*/
void error (const char *msg, int line)
{
	fprintf (stderr, "ERROR [line: %d] %s\n", line, msg);
	fflush (stderr);
	cleanup ();
}