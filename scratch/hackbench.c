From linux-kernel-owner@vger.kernel.org Tue Dec 11 01:11:36 2001
Received: from sgi.com (sgi.engr.sgi.com [192.26.80.37]) by tulip-e185.americas.sgi.com (980427.SGI.8.8.8/SGI-server-1.7) with ESMTP id BAA82859; Tue, 11 Dec 2001 01:11:36 -0600 (CST)
Received: from vger.kernel.org ([199.183.24.194]) 
	by sgi.com (980327.SGI.8.8.8-aspam/980304.SGI-aspam:
       SGI does not authorize the use of its proprietary
       systems or networks for unsolicited or bulk email
       from the Internet.) 
	via ESMTP id XAA08571; Mon, 10 Dec 2001 23:11:32 -0800 (PST)
	mail_from (linux-kernel-owner@vger.kernel.org)
Received: (majordomo@vger.kernel.org) by vger.kernel.org via listexpand
	id <S284902AbRLKHHQ>; Tue, 11 Dec 2001 02:07:16 -0500
Received: (majordomo@vger.kernel.org) by vger.kernel.org
	id <S284914AbRLKHG5>; Tue, 11 Dec 2001 02:06:57 -0500
Received: from [202.135.142.194] ([202.135.142.194]:24588 "EHLO
	wagner.rustcorp.com.au") by vger.kernel.org with ESMTP
	id <S284913AbRLKHGr>; Tue, 11 Dec 2001 02:06:47 -0500
Received: from wagner.rustcorp.com.au ([127.0.0.1] helo=rustcorp.com.au)
	by wagner.rustcorp.com.au with esmtp (Exim 3.32 #1 (Debian))
	id 16Dh0t-0003yl-00; Tue, 11 Dec 2001 18:07:39 +1100
From: Rusty Russell <rusty@rustcorp.com.au>
To: linux-kernel@vger.kernel.org
cc: lse-tech@lists.sourceforge.net
Subject: hackbench: New Multiqueue Scheduler Benchmark
Date: 	Tue, 11 Dec 2001 18:07:39 +1100
Message-Id: <E16Dh0t-0003yl-00@wagner.rustcorp.com.au>
Sender: linux-kernel-owner@vger.kernel.org
Precedence: bulk
X-Mailing-List: 	linux-kernel@vger.kernel.org
Status: RO
Content-Length: 4977

Hi all,

	I've cut down the chat benchmark into a version which doesn't
use threads or semaphores, and called it hackbench:

On a 12-way PPC64:
2.4.17-pre7:
	hackbench 50: Time: 851.469 Time: 847.143 Time: 826.868

2.4.17-pre7-multiqueue-patch:
	hackbench 50: Time: 15.120 Time: 14.766 Time: 15.067

"hackbench 1" creates a group of 20 processes listening on a socket
each, and 20 writers: each of the writers writes 100 messages to each
socket (ie. 20 x 100 messages each).

"hackbench 50" simply runs 50 of these groups in parallel.  You'd
expect it to be 5 times slower than hackbench 10, but without the
multiqueue patch:

10: Time: 10.573 Time: 13.471 Time: 9.289
50: Time: 851.469 Time: 847.143 Time: 826.868

Enjoy,
Rusty.
--
  Anyone who quotes me in their sig is an idiot. -- Rusty Russell.

/* Test groups of 20 processes spraying to 20 receivers */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/poll.h>

#define DATASIZE 100
static unsigned int loops = 100;
static int use_pipes = 0;

static void barf(const char *msg)
{
	fprintf(stderr, "%s (error: %s)\n", msg, strerror(errno));
	exit(1);
}

static void fdpair(int fds[2])
{
	if (use_pipes) {
		if (pipe(fds) == 0)
			return;
	} else {
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0)
			return;
	}
	barf("Creating fdpair");
}

/* Block until we're ready to go */
static void ready(int ready_out, int wakefd)
{
	char dummy;
	struct pollfd pollfd = { .fd = wakefd, .events = POLLIN };

	/* Tell them we're ready. */
	if (write(ready_out, &dummy, 1) != 1)
		barf("CLIENT: ready write");

	/* Wait for "GO" signal */
	if (poll(&pollfd, 1, -1) != 1)
		barf("poll");
}

/* Sender sprays loops messages down each file descriptor */
static void sender(unsigned int num_fds,
		   int out_fd[num_fds],
		   int ready_out,
		   int wakefd)
{
	char data[DATASIZE];
	unsigned int i, j;

	ready(ready_out, wakefd);

	/* Now pump to every receiver. */
	for (i = 0; i < loops; i++) {
		for (j = 0; j < num_fds; j++) {
			int ret, done = 0;

		again:
			ret = write(out_fd[j], data + done, sizeof(data)-done);
			if (ret < 0)
				barf("SENDER: write");
			done += ret;
			if (done < sizeof(data))
				goto again;
		}
	}
}

/* One receiver per fd */
static void receiver(unsigned int num_packets,
		     int in_fd,
		     int ready_out,
		     int wakefd)
{
	unsigned int i;

	/* Wait for start... */
	ready(ready_out, wakefd);

	/* Receive them all */
	for (i = 0; i < num_packets; i++) {
		char data[DATASIZE];
		int ret, done = 0;

	again:
		ret = read(in_fd, data + done, DATASIZE - done);
		if (ret < 0)
			barf("SERVER: read");
		done += ret;
		if (done < DATASIZE)
			goto again;
	}
}

/* One group of senders and receivers */
static unsigned int group(unsigned int num_fds,
			  int ready_out,
			  int wakefd)
{
	unsigned int i;
	unsigned int out_fds[num_fds];

	for (i = 0; i < num_fds; i++) {
		int fds[2];

		/* Create the pipe between client and server */
		fdpair(fds);

		/* Fork the receiver. */
		switch (fork()) {
		case -1: barf("fork()");
		case 0:
			close(fds[1]);
			receiver(num_fds*loops, fds[0], ready_out, wakefd);
			exit(0);
		}

		out_fds[i] = fds[1];
		close(fds[0]);
	}

	/* Now we have all the fds, fork the senders */
	for (i = 0; i < num_fds; i++) {
		switch (fork()) {
		case -1: barf("fork()");
		case 0:
			sender(num_fds, out_fds, ready_out, wakefd);
			exit(0);
		}
	}

	/* Close the fds we have left */
	for (i = 0; i < num_fds; i++)
		close(out_fds[i]);

	/* Return number of children to reap */
	return num_fds * 2;
}

int main(int argc, char *argv[])
{
	unsigned int i, num_groups, total_children;
	struct timeval start, stop, diff;
	unsigned int num_fds = 20;
	int readyfds[2], wakefds[2];
	char dummy;

	if (argv[1] && strcmp(argv[1], "-pipe") == 0) {
		use_pipes = 1;
		argc--;
		argv++;
	}

	if (argc != 2 || (num_groups = atoi(argv[1])) == 0)
		barf("Usage: hackbench [-pipe] <num groups>\n");

	fdpair(readyfds);
	fdpair(wakefds);

	total_children = 0;
	for (i = 0; i < num_groups; i++)
		total_children += group(num_fds, readyfds[1], wakefds[0]);

	/* Wait for everyone to be ready */
	for (i = 0; i < total_children; i++)
		if (read(readyfds[0], &dummy, 1) != 1)
			barf("Reading for readyfds");

	gettimeofday(&start, NULL);

	/* Kick them off */
	if (write(wakefds[1], &dummy, 1) != 1)
		barf("Writing to start them");

	/* Reap them all */
	for (i = 0; i < total_children; i++) {
		int status;
		wait(&status);
		if (!WIFEXITED(status))
			exit(1);
	}

	gettimeofday(&stop, NULL);

	/* Print time... */
	timersub(&stop, &start, &diff);
	printf("Time: %lu.%03lu\n", diff.tv_sec, diff.tv_usec/1000);
	exit(0);
}
-
To unsubscribe from this list: send the line "unsubscribe linux-kernel" in
the body of a message to majordomo@vger.kernel.org
More majordomo info at  http://vger.kernel.org/majordomo-info.html
Please read the FAQ at  http://www.tux.org/lkml/


