/* TODO: proper output */

/*
 *
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 *  FILE        : pth_str03.c
 *  DESCRIPTION : create a tree of threads does calculations, and
 *                returns result to parent
 *  HISTORY:
 *    05/l6/2001 Paul Larson (plars@us.ibm.com)
 *      -Ported
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include "test.h"

#define MAXTHREADS 1000

/* Type definition */
struct kid_info {
	long sum;		/* sum of childrens indexes plus our own */
	int index;		/* our index into the array */
	int status;		/* return status of this thread */
	int child_count;	/* Count of children created */
	int talk_count;		/* Count of siblings that we talked to */
	pthread_t *threads;	/* dynamic array of thread id of kids */
	pthread_mutex_t talk_mutex;	/* mutex for the talk_count */
	pthread_mutex_t child_mutex;	/* mutex for the child_count */
	pthread_cond_t talk_condvar;	/* condition variable for talk_count */
	pthread_cond_t child_condvar;	/* condition variable for child_count */
	struct kid_info **child_ptrs;	/* dynamic array of ptrs to kids */
};
typedef struct kid_info c_info;

/* Global variables */
int depth = 3;
int breadth = 4;
int timeout = 30;		/* minutes */
int cdepth;			/* current depth */
int debug = 0;

c_info *child_info;		/* pointer to info array */
int node_count;			/* number of nodes created so far */
pthread_mutex_t node_mutex;	/* mutex for the node_count */
pthread_cond_t node_condvar;	/* condition variable for node_count */
pthread_attr_t attr;		/* attributes for created threads */

int num_nodes(int, int);
int synchronize_children(c_info *);
void *doit(void *);

char *TCID = "pth_str03";
int TST_TOTAL = 1;

void testexit(int value)
{
	if (value == 0)
		tst_resm(TPASS, "Test passed");
	else
		tst_resm(TFAIL, "Test failed");

	exit(value);
}

/*--------------------------------------------------------------------------------*
 * parse_args
 *
 * Parse command line arguments.  Any errors cause the program to exit
 * at this point.
 *--------------------------------------------------------------------------------*/
static void parse_args(int argc, char *argv[])
{
	int opt, errflag = 0;
	int bflag = 0, dflag = 0, tflag = 0;

	while ((opt = getopt(argc, argv, "b:d:t:D?")) != EOF) {
		switch (opt) {
		case 'b':
			if (bflag)
				errflag++;
			else {
				bflag++;
				breadth = atoi(optarg);
				if (breadth <= 0)
					errflag++;
			}
			break;
		case 'd':
			if (dflag)
				errflag++;
			else {
				dflag++;
				depth = atoi(optarg);
				if (depth <= 0)
					errflag++;
			}
			break;
		case 't':
			if (tflag)
				errflag++;
			else {
				tflag++;
				timeout = atoi(optarg);
				if (timeout <= 0)
					errflag++;
			}
			break;
		case 'D':
			debug = 1;
			break;
		case '?':
		default:
			errflag++;
			break;
		}
	}

	if (errflag) {
		fprintf(stderr,
			"usage: %s [-b <num>] [-d <num>] [-t <num>] [-D]",
			argv[0]);
		fprintf(stderr, "where:\n");
		fprintf(stderr, "\t-b <num>\tbreadth of child nodes\n");
		fprintf(stderr, "\t-d <num>\tdepth of child nodes\n");
		fprintf(stderr,
			"\t-t <num>\ttimeout for child communication (in minutes)\n");
		fprintf(stderr, "\t-D\tdebug mode\n");
		testexit(1);
	}

}

/*--------------------------------------------------------------------------------*
 * num_nodes
 *
 * Caculate the number of child nodes for a given breadth and depth tree.
 *--------------------------------------------------------------------------------*/
int num_nodes(int b, int d)
{
	int n, sum = 1, partial_exp = 1;

	/*
	 * The total number of children = sum ( b ** n )
	 *                                n=0->d
	 * Since b ** 0 == 1, and it's hard to compute that kind of value
	 * in this simplistic loop, we start sum at 1 (above) to compensate
	 * and do the computations from 1->d below.
	 */
	for (n = 1; n <= d; n++) {
		partial_exp *= b;
		sum += partial_exp;
	}

	return (sum);
}

/*--------------------------------------------------------------------------------*
 * synchronize_children
 *
 * Register the child with the parent and then wait for all of the children
 * at the same level to register also.  Return when everything is synched up.
 *--------------------------------------------------------------------------------*/
int synchronize_children(c_info * parent)
{
	int my_index, rc;
	c_info *info_p;
	struct timespec timer;

	if (debug) {
		printf("trying to lock node_mutex\n");
		fflush(stdout);
	}

	/* Lock the node_count mutex to we can safely increment it.  We
	 * will unlock it when we broadcast that all of our siblings have
	 * been created or when we block waiting for that broadcast.  */
	pthread_mutex_lock(&node_mutex);
	my_index = node_count++;

	tst_resm(TINFO, "thread %d started", my_index);

	/* Get a pointer into the array of thread structures which will be "me". */
	info_p = &child_info[my_index];
	info_p->index = my_index;
	info_p->sum = (long)my_index;

	if (debug) {
		printf("thread %d info_p=%p\n", my_index, info_p);
		fflush(stdout);
	}

	/* Register with parent bumping the parent's child_count variable.
	 * Make sure we have exclusive access to that variable before we
	 * do the increment.  */
	if (debug) {
		printf("thread %d locking child_mutex %p\n", my_index,
		       &parent->child_mutex);
		fflush(stdout);
	}
	pthread_mutex_lock(&parent->child_mutex);
	if (debug) {
		printf("thread %d bumping child_count (currently %d)\n",
		       my_index, parent->child_count);
		fflush(stdout);
	}
	parent->child_ptrs[parent->child_count++] = info_p;
	if (debug) {
		printf("thread %d unlocking child_mutex %p\n", my_index,
		       &parent->child_mutex);
		fflush(stdout);
	}
	pthread_mutex_unlock(&parent->child_mutex);

	if (debug) {
		printf("thread %d node_count = %d\n", my_index, node_count);
		printf("expecting %d nodes\n", num_nodes(breadth, cdepth));
		fflush(stdout);
	}

	/* Have all the nodes at our level in the tree been created yet?
	 * If so, then send out a broadcast to wake everyone else up (to let
	 * them know they can now create their children (if they are not
	 * leaf nodes)).  Otherwise, go to sleep waiting for the broadcast.  */
	if (node_count == num_nodes(breadth, cdepth)) {

		/* Increase the current depth variable, as the tree is now
		 * fully one level taller.  */
		if (debug) {
			printf("thread %d doing cdepth++ (%d)\n", my_index,
			       cdepth);
			fflush(stdout);
		}
		cdepth++;

		if (debug) {
			printf("thread %d sending child_mutex broadcast\n",
			       my_index);
			fflush(stdout);
		}

		/* Since all of our siblings have been created, this level of
		 * the tree is now allowed to continue its work, so wake up the
		 * rest of the siblings.  */
		pthread_cond_broadcast(&node_condvar);

	} else {

		/* In this case, not all of our siblings at this level of the
		 * tree have been created, so go to sleep and wait for the
		 * broadcast on node_condvar.  */
		if (debug) {
			printf("thread %d waiting for siblings to register\n",
			       my_index);
			fflush(stdout);
		}
		time(&timer.tv_sec);
		timer.tv_sec += (unsigned long)timeout *60;
		timer.tv_nsec = (unsigned long)0;
		if ((rc = pthread_cond_timedwait(&node_condvar, &node_mutex,
						 &timer))) {
			tst_resm(TINFO, "pthread_cond_timedwait (sync) %d: %s",
				 my_index, strerror(rc));
			testexit(2);
		}

		if (debug) {
			printf("thread %d is now unblocked\n", my_index);
			fflush(stdout);
		}

	}

	/* Unlock the node_mutex lock, as this thread is finished initializing.  */
	if (debug) {
		printf("thread %d unlocking node_mutex\n", my_index);
		fflush(stdout);
	}
	pthread_mutex_unlock(&node_mutex);
	if (debug) {
		printf("thread %d unlocked node_mutex\n", my_index);
		fflush(stdout);
	}

	if (debug) {
		printf("synchronize_children returning %d\n", my_index);
		fflush(stdout);
	}

	return (my_index);
}

/*--------------------------------------------------------------------------------*
 * doit
 *
 * Do it.
 *--------------------------------------------------------------------------------*/
void *doit(void *param)
{
	c_info *parent = (c_info *) param;
	int rc, child, my_index;
	void *status;
	c_info *info_p;
	struct timespec timer;

	if (parent != NULL) {
		/* Synchronize with our siblings so that all the children at
		 * a given level have been created before we allow those children
		 * to spawn new ones (or do anything else for that matter).  */
		if (debug) {
			printf("non-root child calling synchronize_children\n");
			fflush(stdout);
		}
		my_index = synchronize_children(parent);
		if (debug) {
			printf("non-root child has been assigned index %d\n",
			       my_index);
			fflush(stdout);
		}
	} else {
		/* The first thread has no one with which to synchronize, so
		 * set some initial values for things.  */
		if (debug) {
			printf("root child\n");
			fflush(stdout);
		}
		cdepth = 1;
		my_index = 0;
		node_count = 1;
	}

	/* Figure out our place in the pthread array.  */
	info_p = &child_info[my_index];

	if (debug) {
		printf("thread %d getting to heart of doit.\n", my_index);
		printf("info_p=%p, cdepth=%d, depth=%d\n", info_p, cdepth,
		       depth);
		fflush(stdout);
	}

	if (cdepth <= depth) {
		/* Since the tree is not yet complete (it is not yet tall enough),
		 * we need to create another level of children.  */

		tst_resm(TINFO, "thread %d creating kids, cdepth=%d", my_index,
			 cdepth);

		/* Create breadth children.  */
		for (child = 0; child < breadth; child++) {
			if ((rc =
			     pthread_create(&(info_p->threads[child]), &attr,
					    doit, (void *)info_p))) {
				tst_resm(TINFO, "pthread_create (doit): %s",
					 strerror(rc));
				testexit(3);
			} else {
				if (debug) {
					printf
					    ("pthread_create made thread %p\n",
					     &(info_p->threads[child]));
					fflush(stdout);
				}
			}
		}

		if (debug) {
			printf("thread %d waits on kids, cdepth=%d\n", my_index,
			       cdepth);
			fflush(stdout);
		}

		/* Wait for our children to finish before we exit ourselves.  */
		for (child = 0; child < breadth; child++) {
			if (debug) {
				printf("attempting join on thread %p\n",
				       &(info_p->threads[child]));
				fflush(stdout);
			}
			if ((rc =
			     pthread_join((info_p->threads[child]), &status))) {
				if (debug) {
					printf
					    ("join failed on thread %d, status addr=%p: %s\n",
					     my_index, status, strerror(rc));
					fflush(stdout);
				}
				testexit(4);
			} else {
				if (debug) {
					printf("thread %d joined child %d ok\n",
					       my_index, child);
					fflush(stdout);
				}

				/* Add all childrens indexes to your index value */
				info_p->sum += info_p->child_ptrs[child]->sum;
				tst_resm(TINFO,
					 "thread %d adding child thread %d to sum = %ld",
					 my_index,
					 info_p->child_ptrs[child]->index,
					 (long int)info_p->sum);
			}
		}

	} else {

		/* This is the leaf node case.  These children don't create
		 * any kids; they just talk amongst themselves.  */
		tst_resm(TINFO, "thread %d is a leaf node, depth=%d", my_index,
			 cdepth);

		/* Talk to siblings (children of the same parent node).  */
		if (breadth > 1) {

			for (child = 0; child < breadth; child++) {
				/* Don't talk to yourself.  */
				if (parent->child_ptrs[child] != info_p) {
					if (debug) {
						printf
						    ("thread %d locking talk_mutex\n",
						     my_index);
						fflush(stdout);
					}
					pthread_mutex_lock(&
							   (parent->
							    child_ptrs[child]->
							    talk_mutex));
					if (++parent->child_ptrs[child]->
					    talk_count == (breadth - 1)) {
						if (debug) {
							printf
							    ("thread %d talk siblings\n",
							     my_index);
							fflush(stdout);
						}
						if ((rc =
						     pthread_cond_broadcast
						     (&parent->
						      child_ptrs[child]->
						      talk_condvar))) {
							tst_resm(TINFO,
								 "pthread_cond_broadcast: %s",
								 strerror(rc));
							testexit(5);
						}
					}
					if (debug) {
						printf
						    ("thread %d unlocking talk_mutex\n",
						     my_index);
						fflush(stdout);
					}
					pthread_mutex_unlock(&
							     (parent->
							      child_ptrs
							      [child]->
							      talk_mutex));
				}
			}

			/* Wait until the breadth - 1 siblings have contacted us.  */
			if (debug) {
				printf("thread %d waiting for talk siblings\n",
				       my_index);
				fflush(stdout);
			}

			pthread_mutex_lock(&info_p->talk_mutex);
			if (info_p->talk_count < (breadth - 1)) {
				time(&timer.tv_sec);
				timer.tv_sec += (unsigned long)timeout *60;
				timer.tv_nsec = (unsigned long)0;
				if ((rc =
				     pthread_cond_timedwait(&info_p->
							    talk_condvar,
							    &info_p->talk_mutex,
							    &timer))) {
					tst_resm(TINFO,
						 "pthread_cond_timedwait (leaf) %d: %s",
						 my_index, strerror(rc));
					testexit(6);
				}
			}
			pthread_mutex_unlock(&info_p->talk_mutex);

		}

	}

	/* Our work is done.  We're outta here. */
	tst_resm(TINFO, "thread %d exiting, depth=%d, status=%d, addr=%p",
		 my_index, cdepth, info_p->status, info_p);

	pthread_exit(0);
}

/*--------------------------------------------------------------------------------*
 * main
 *--------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int rc, ind, total;
	pthread_t root_thread;

	parse_args(argc, argv);

	/* Initialize node mutex.  */
	if ((rc = pthread_mutex_init(&node_mutex, NULL))) {
		tst_resm(TINFO, "pthread_mutex_init(node_mutex): %s",
			 strerror(rc));
		testexit(7);
	}

	/* Initialize node condition variable.  */
	if ((rc = pthread_cond_init(&node_condvar, NULL))) {
		tst_resm(TINFO, "pthread_cond_init(node_condvar): %s",
			 strerror(rc));
		testexit(8);
	}

	/* Allocate pthread info structure array.  */
	if ((total = num_nodes(breadth, depth)) > MAXTHREADS) {
		tst_resm(TINFO, "Can't create %d threads; max is %d.",
			 total, MAXTHREADS);
		testexit(9);
	}
	tst_resm(TINFO, "Allocating %d nodes.", total);
	if ((child_info = malloc(total * sizeof(c_info))) == NULL) {
		perror("malloc child_info");
		testexit(10);
	}
	memset(child_info, 0x00, total * sizeof(c_info));

	if (debug) {
		printf("Initializing array for %d children\n", total);
		fflush(stdout);
	}

	/* Allocate array of pthreads descriptors and initialize variables.  */
	for (ind = 0; ind < total; ind++) {

		if ((child_info[ind].threads = malloc(breadth * sizeof(pthread_t))) ==
		    NULL) {
			perror("malloc threads");
			testexit(11);
		}
		memset(child_info[ind].threads, 0x00,
		       breadth * sizeof(pthread_t));

		if ((child_info[ind].child_ptrs = malloc(breadth * sizeof(c_info *))) == NULL) {
			perror("malloc child_ptrs");
			testexit(12);
		}
		memset(child_info[ind].child_ptrs, 0x00,
		       breadth * sizeof(c_info *));

		if ((rc =
		     pthread_mutex_init(&child_info[ind].child_mutex, NULL))) {
			tst_resm(TINFO, "pthread_mutex_init child_mutex: %s",
				 strerror(rc));
			testexit(13);
		}

		if ((rc =
		     pthread_mutex_init(&child_info[ind].talk_mutex, NULL))) {
			tst_resm(TINFO, "pthread_mutex_init talk_mutex: %s",
				 strerror(rc));
			testexit(14);
		}

		if ((rc =
		     pthread_cond_init(&child_info[ind].child_condvar, NULL))) {
			tst_resm(TINFO, "pthread_cond_init child_condvar: %s",
				 strerror(rc));
			testexit(15);
		}

		if ((rc =
		     pthread_cond_init(&child_info[ind].talk_condvar, NULL))) {
			tst_resm(TINFO, "pthread_cond_init talk_condvar: %s",
				 strerror(rc));
			testexit(16);
		}

		if (debug) {
			printf("Successfully initialized child %d.\n", ind);
			fflush(stdout);
		}

	}

	tst_resm(TINFO,
		 "Creating root thread attributes via pthread_attr_init.");

	if ((rc = pthread_attr_init(&attr))) {
		tst_resm(TINFO, "pthread_attr_init: %s", strerror(rc));
		testexit(17);
	}

	/* Make sure that all the thread children we create have the
	 * PTHREAD_CREATE_JOINABLE attribute.  If they don't, then the
	 * pthread_join call will sometimes fail and cause mass confusion.  */
	if ((rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE))) {
		tst_resm(TINFO, "pthread_attr_setdetachstate: %s",
			 strerror(rc));
		testexit(18);
	}

	tst_resm(TINFO, "Creating root thread via pthread_create.");

	if ((rc = pthread_create(&root_thread, &attr, doit, NULL))) {
		tst_resm(TINFO, "pthread_create: %s", strerror(rc));
		testexit(19);
	}

	if (debug) {
		printf("Doing pthread_join.\n");
		fflush(stdout);
	}

	/* Wait for the root child to exit.  */
	if ((rc = pthread_join(root_thread, NULL))) {
		tst_resm(TINFO, "pthread_join: %s", strerror(rc));
		testexit(20);
	}

	if (debug) {
		printf("About to pthread_exit.\n");
		fflush(stdout);
	}

	tst_resm(TINFO, "The sum of tree (breadth %d, depth %d) is %ld",
		 breadth, depth, child_info[0].sum);
	testexit(0);

	exit(0);
}
