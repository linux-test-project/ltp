/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2007, 2008
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
 *
 * NAME
 *      sched_football.c
 *
 * DESCRIPTION
 *      This is a scheduler test that uses a football analogy.
 *      The premise is that we want to make sure that lower priority threads
 *      don't run while we have runnable higher priority threads.
 *      The offense is trying to increment the balls position, while the
 *      defense is trying to block that from happening.
 *      And the ref (highest priority thread) will blow the wistle if the
 *      ball moves. Finally, we have crazy fans (higer prority) that try to
 *      distract the defense by occasionally running onto the field.
 *
 *      Steps:
 *       - Create NR_CPU offense threads (lower priority)
 *       - Create NR_CPU defense threads (mid priority)
 *       - Create 2*NR_CPU fan threads (high priority)
 *       - Create a referee thread (highest priority)
 *       - Once everyone is on the field, the offense thread spins incrementing
 *         the value of 'the_ball'. The defense thread tries to block the ball
 *         by never letting the offense players get the CPU (it just spins).
 *         The crazy fans sleep a bit, then jump the rail and run across the
 *         field, disrupting the players on the field.
 *       - The refree threads wakes up regularly to check if the game is over :)
 *       - In the end, if the value of 'the_ball' is >0, the test is considered
 *         to have failed.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      John Stultz <johnstul@xxxxxxxxx >
 *
 * HISTORY
 *     2006-03-16 Reduced verbosity, non binary failure reporting, removal of
 *		crazy_fans thread, added game_length argument by Darren Hart.
 *     2007-08-01 Remove all thread cleanup in favor of simply exiting.Various
 *		bugfixes and cleanups. -- Josh Triplett
 *     2009-06-23 Simplified atomic startup mechanism, avoiding thundering herd
 *		scheduling at the beginning of the game. -- Darren Hart
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <librttest.h>
#include <tst_atomic.h>
#define TST_NO_DEFAULT_MAIN
#include <tst_timer.h>


#define DEF_GAME_LENGTH 5

/* Here's the position of the ball */
static int the_ball;

static int players_per_team = 0;
static int game_length = DEF_GAME_LENGTH;
static int players_ready;

void usage(void)
{
	rt_help();
	printf("sched_football specific options:\n");
	printf("  -nPLAYERS     players per team (defaults to num_cpus)\n");
	printf("  -lGAME_LENGTH game length in seconds (defaults to %d s)\n",
	       DEF_GAME_LENGTH);
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'n':
		players_per_team = atoi(v);
		break;
	case 'l':
		game_length = atoi(v);
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

#define SPIN_TIME_NS 200000000ULL
#define SLEEP_TIME_NS 50000000ULL
/* These are fans running across the field. They're trying to interrupt/distract everyone */
void *thread_fan(void *arg)
{
	prctl(PR_SET_NAME, "crazy_fan", 0, 0, 0);
	tst_atomic_add_return(1, &players_ready);
	/*occasionally wake up and run across the field */
	while (1) {
		struct timespec start, stop;
		nsec_t nsec;

		start.tv_sec = 0;
		start.tv_nsec = SLEEP_TIME_NS;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &start, NULL);
		clock_gettime(CLOCK_MONOTONIC, &start);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		nsec = tst_timespec_diff_ns(stop, start);
		while (nsec < SPIN_TIME_NS) {
			clock_gettime(CLOCK_MONOTONIC, &stop);
			nsec = tst_timespec_diff_ns(stop, start);
		}
	}
	return NULL;
}

/* This is the defensive team. They're trying to block the offense */
void *thread_defense(void *arg)
{
	prctl(PR_SET_NAME, "defense", 0, 0, 0);
	tst_atomic_add_return(1, &players_ready);
	/*keep the ball from being moved */
	while (1) {
	}
	return NULL;
}

/* This is the offensive team. They're trying to move the ball */
void *thread_offense(void *arg)
{
	prctl(PR_SET_NAME, "offense", 0, 0, 0);
	tst_atomic_add_return(1, &players_ready);
	while (1) {
		tst_atomic_add_return(1, &the_ball); /* move the ball ahead one yard */
	}
	return NULL;
}

int referee(int game_length)
{
	struct timeval start, now;
	int final_ball;

	prctl(PR_SET_NAME, "referee", 0, 0, 0);
	printf("Game On (%d seconds)!\n", game_length);

	gettimeofday(&start, NULL);
	now = start;

	/* Start the game! */
	tst_atomic_store(0, &the_ball);

	/* Watch the game */
	while ((now.tv_sec - start.tv_sec) < game_length) {
		sleep(1);
		gettimeofday(&now, NULL);
	}
	final_ball = tst_atomic_load(&the_ball);
	/* Blow the whistle */
	printf("Game Over!\n");
	printf("Final ball position: %d\n", final_ball);
	return final_ball != 0;
}

int main(int argc, char *argv[])
{
	struct sched_param param;
	int priority;
	int i;
	int result;
	setup();

	rt_init("n:l:h", parse_args, argc, argv);

	if (players_per_team == 0)
		players_per_team = sysconf(_SC_NPROCESSORS_ONLN);

	tst_atomic_store(0, &players_ready);

	printf("Running with: players_per_team=%d game_length=%d\n",
	       players_per_team, game_length);

	/* We're the ref, so set our priority right */
	param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 80;
	sched_setscheduler(0, SCHED_FIFO, &param);

	/*
	 * Start the offense
	 * They are lower priority than defense, so they must be started first.
	 */
	priority = 15;
	printf("Starting %d offense threads at priority %d\n",
	       players_per_team, priority);
	for (i = 0; i < players_per_team; i++)
		create_fifo_thread(thread_offense, NULL, priority);

	/* Wait for the offense threads to start */
	while (tst_atomic_load(&players_ready) < players_per_team)
		usleep(100);

	/* Start the defense */
	priority = 30;
	printf("Starting %d defense threads at priority %d\n",
	       players_per_team, priority);
	for (i = 0; i < players_per_team; i++)
		create_fifo_thread(thread_defense, NULL, priority);

	/* Wait for the defense threads to start */
	while (tst_atomic_load(&players_ready) < players_per_team * 2)
		usleep(100);

	/* Start the crazy fans*/
	priority = 50;
	printf("Starting %d fan threads at priority %d\n",
	       players_per_team, priority);
	for (i = 0; i < players_per_team*2; i++)
		create_fifo_thread(thread_fan, NULL, priority);

	/* Wait for the crazy fan threads to start */
	while (tst_atomic_load(&players_ready) < players_per_team * 4)
		usleep(100);

	/* let things get into steady state */
	sleep(2);
	/* Ok, everyone is on the field, bring out the ref */
	printf("Starting referee thread\n");
	result = referee(game_length);
	printf("Result: %s\n", result ? "FAIL" : "PASS");
	return result;

}
