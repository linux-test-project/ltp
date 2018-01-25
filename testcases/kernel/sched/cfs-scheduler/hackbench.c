/******************************************************************************/
/* Copyright Rusty Russell,                                                   */
/* Copyright Pierre Peiffer                                                   */
/* Copyright Zhang, Yanmin,                                                   */
/* Copyright Ingo Molnar,                                                     */
/* Copyright Arjan van de Ven,                                                */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        hackbench.c                                                   */
/*                                                                            */
/* Description: hackbench tests the Linux scheduler. Test groups of 20        */
/*              processes spraying to 20 receivers                            */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   hackbench01 and hackbench02                                   */
/*                                                                            */
/* Test Assertion:                                                            */
/*                                                                            */
/* Author(s):   Rusty Russell <rusty@rustcorp.com.au>,                        */
/*              Pierre Peiffer <pierre.peiffer@bull.net>,                     */
/*              Ingo Molnar <mingo@elte.hu>,                                  */
/*              Arjan van de Ven <arjan@infradead.org>,                       */
/*              "Zhang, Yanmin" <yanmin_zhang@linux.intel.com>,               */
/*              Nathan Lynch <ntl@pobox.com>                                  */
/*                                                                            */
/* History:     Included into LTP                                             */
/*                  - June 26 2008 - Subrata Modak<subrata@linux.vnet.ibm.com>*/
/*                                                                            */
/******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <limits.h>

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }
#define DATASIZE 100
static struct sender_context **snd_ctx_tab;	/*Table for sender context pointers. */
static struct receiver_context **rev_ctx_tab;	/*Table for receiver context pointers. */
static int gr_num = 0;		/*For group calculation */
static unsigned int loops = 100;
/*
 * 0 means thread mode and others mean process (default)
 */
static unsigned int process_mode = 1;

static int use_pipes = 0;

struct sender_context {
	unsigned int num_fds;
	int ready_out;
	int wakefd;
	int out_fds[0];
};

struct receiver_context {
	unsigned int num_packets;
	int in_fds[2];
	int ready_out;
	int wakefd;
};

static void barf(const char *msg)
{
	fprintf(stderr, "%s (error: %s)\n", msg, strerror(errno));
	exit(1);
}

static void print_usage_exit(void)
{
	printf
	    ("Usage: hackbench [-pipe] <num groups> [process|thread] [loops]\n");
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
	struct pollfd pollfd = {.fd = wakefd,.events = POLLIN };

	/* Tell them we're ready. */
	if (write(ready_out, &dummy, 1) != 1)
		barf("CLIENT: ready write");

	/* Wait for "GO" signal */
	if (poll(&pollfd, 1, -1) != 1)
		barf("poll");
}

/* Sender sprays loops messages down each file descriptor */
static void *sender(struct sender_context *ctx)
{
	char data[DATASIZE];
	unsigned int i, j;

	ready(ctx->ready_out, ctx->wakefd);

	/* Now pump to every receiver. */
	for (i = 0; i < loops; i++) {
		for (j = 0; j < ctx->num_fds; j++) {
			int ret, done = 0;

again:
			ret =
			    write(ctx->out_fds[j], data + done,
				  sizeof(data) - done);
			if (ret < 0)
				barf("SENDER: write");
			done += ret;
			if (done < sizeof(data))
				goto again;
		}
	}

	return NULL;
}

/* One receiver per fd */
static void *receiver(struct receiver_context *ctx)
{
	unsigned int i;

	if (process_mode)
		close(ctx->in_fds[1]);

	/* Wait for start... */
	ready(ctx->ready_out, ctx->wakefd);

	/* Receive them all */
	for (i = 0; i < ctx->num_packets; i++) {
		char data[DATASIZE];
		int ret, done = 0;

again:
		ret = read(ctx->in_fds[0], data + done, DATASIZE - done);
		if (ret < 0)
			barf("SERVER: read");
		done += ret;
		if (done < DATASIZE)
			goto again;
	}

	return NULL;
}

pthread_t create_worker(void *ctx, void *(*func) (void *))
{
	pthread_attr_t attr;
	pthread_t childid;
	int err;

	if (process_mode) {
		/* process mode */
		/* Fork the receiver. */
		switch (fork()) {
		case -1:
			barf("fork()");
		case 0:
			(*func) (ctx);
			exit(0);
		}

		return (pthread_t) 0;
	}

	if (pthread_attr_init(&attr) != 0)
		barf("pthread_attr_init:");

#ifndef __ia64__
	if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN) != 0)
		barf("pthread_attr_setstacksize");
#endif

	if ((err = pthread_create(&childid, &attr, func, ctx)) != 0) {
		fprintf(stderr, "pthread_create failed: %s (%d)\n",
			strerror(err), err);
		exit(-1);
	}
	return (childid);
}

void reap_worker(pthread_t id)
{
	int status;

	if (process_mode) {
		/* process mode */
		wait(&status);
		if (!WIFEXITED(status))
			exit(1);
	} else {
		void *status;

		pthread_join(id, &status);
	}
}

/* One group of senders and receivers */
static unsigned int group(pthread_t * pth,
			  unsigned int num_fds, int ready_out, int wakefd)
{
	unsigned int i;
	struct sender_context *snd_ctx = malloc(sizeof(struct sender_context) + num_fds * sizeof(int));
	if (!snd_ctx)
		barf("malloc()");
	else
		snd_ctx_tab[gr_num] = snd_ctx;

	for (i = 0; i < num_fds; i++) {
		int fds[2];
		struct receiver_context *ctx = malloc(sizeof(*ctx));

		if (!ctx)
			barf("malloc()");
		else
			rev_ctx_tab[gr_num * num_fds + i] = ctx;

		/* Create the pipe between client and server */
		fdpair(fds);

		ctx->num_packets = num_fds * loops;
		ctx->in_fds[0] = fds[0];
		ctx->in_fds[1] = fds[1];
		ctx->ready_out = ready_out;
		ctx->wakefd = wakefd;

		pth[i] = create_worker(ctx, (void *)(void *)receiver);

		snd_ctx->out_fds[i] = fds[1];
		if (process_mode)
			close(fds[0]);
	}

	/* Now we have all the fds, fork the senders */
	for (i = 0; i < num_fds; i++) {
		snd_ctx->ready_out = ready_out;
		snd_ctx->wakefd = wakefd;
		snd_ctx->num_fds = num_fds;

		pth[num_fds + i] =
		    create_worker(snd_ctx, (void *)(void *)sender);
	}

	/* Close the fds we have left */
	if (process_mode)
		for (i = 0; i < num_fds; i++)
			close(snd_ctx->out_fds[i]);

	gr_num++;
	/* Return number of children to reap */
	return num_fds * 2;
}

int main(int argc, char *argv[])
{
	unsigned int i, j, num_groups = 10, total_children;
	struct timeval start, stop, diff;
	unsigned int num_fds = 20;
	int readyfds[2], wakefds[2];
	char dummy;
	pthread_t *pth_tab;

	if (argv[1] && strcmp(argv[1], "-pipe") == 0) {
		use_pipes = 1;
		argc--;
		argv++;
	}

	if (argc >= 2 && (num_groups = atoi(argv[1])) == 0)
		print_usage_exit();

	printf("Running with %d*40 (== %d) tasks.\n",
	       num_groups, num_groups * 40);

	fflush(NULL);

	if (argc > 2) {
		if (!strcmp(argv[2], "process"))
			process_mode = 1;
		else if (!strcmp(argv[2], "thread"))
			process_mode = 0;
		else
			print_usage_exit();
	}

	if (argc > 3)
		loops = atoi(argv[3]);

	pth_tab = malloc(num_fds * 2 * num_groups * sizeof(pthread_t));
	snd_ctx_tab = malloc(num_groups * sizeof(void *));
	rev_ctx_tab = malloc(num_groups * num_fds * sizeof(void *));
	if (!pth_tab || !snd_ctx_tab || !rev_ctx_tab)
		barf("main:malloc()");

	fdpair(readyfds);
	fdpair(wakefds);

	total_children = 0;
	for (i = 0; i < num_groups; i++)
		total_children +=
		    group(pth_tab + total_children, num_fds, readyfds[1],
			  wakefds[0]);

	/* Wait for everyone to be ready */
	for (i = 0; i < total_children; i++)
		if (read(readyfds[0], &dummy, 1) != 1)
			barf("Reading for readyfds");

	gettimeofday(&start, NULL);

	/* Kick them off */
	if (write(wakefds[1], &dummy, 1) != 1)
		barf("Writing to start them");

	/* Reap them all */
	for (i = 0; i < total_children; i++)
		reap_worker(pth_tab[i]);

	gettimeofday(&stop, NULL);

	/* Print time... */
	timersub(&stop, &start, &diff);
	printf("Time: %lu.%03lu\n", diff.tv_sec, diff.tv_usec / 1000);

	/* free the memory */
	for (i = 0; i < num_groups; i++) {
		for (j = 0; j < num_fds; j++) {
			SAFE_FREE(rev_ctx_tab[i * num_fds + j])
		}
		SAFE_FREE(snd_ctx_tab[i]);
	}
	SAFE_FREE(pth_tab);
	SAFE_FREE(snd_ctx_tab);
	SAFE_FREE(rev_ctx_tab);
	exit(0);
}
