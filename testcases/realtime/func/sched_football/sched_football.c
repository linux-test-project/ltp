/******************************************************************************
 *
 *   Copyright  International Business Machines  Corp., 2007
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
 *
 * NAME
 *      sched_football.c
 *
 * DESCRIPTION
 *      This is a scheduler test that uses a football analogy.
 *      The premise is that we want to make sure that lower priority threads
 *      (defensive team). The offense is trying to increment the balls position,
 *      while the defense is trying to block that from happening.
 *      And the ref (highest priority thread) will blow the wistle if the
 *      ball moves. Finally, we have crazy fans (higer prority) that try to
 *      distract the defense by occasionally running onto the field.
 *       Steps:
 *          - Create a fixed number of offense threads (lower priority)
 *          - Create a referee thread (highest priority)
 *          - Once everyone is on the field, the offense thread increments the value of
 *            'the_ball' and yields. The defense thread tries to block the ball by never
 *            letting the offense players get the CPU (it just does a sched_yield)
 *          - The refree threads wakes up regularly to check if the game is over :)
 *          - In the end, if the value of 'the_ball' is >0, the test is considered to
 *            have failed.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *      Use "-j" to enable jvm simulator.
 *
 * AUTHOR
 *      John Stultz <johnstul@xxxxxxxxx >
 *
 * HISTORY
 *     2006-03-16 Reduced verbosity, non binary failure reporting, removal of
 *     crazy_fans thread, added game_length argument by Darren Hart.
 * 	2007-08-01 Remove all thread cleanup in favor of simply exiting.  Various
 * 	bugfixes and cleanups. -- Josh Triplett
 *
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
#include <sys/time.h>
#include <librttest.h>
#include <libjvmsim.h>

#define DEF_GAME_LENGTH 5

/* Here's the position of the ball */
volatile int the_ball;

/* Keep track of who's on the field */
volatile int offense_count;
volatile int defense_count;

/* simple mutex for our atomic increments */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int run_jvmsim=0;

void usage(void)
{
        rt_help();
        printf("testpi-1 specific options:\n");
        printf("  -j            enable jvmsim\n");
}

int parse_args(int c, char *v)
{

        int handled = 1;
        switch (c) {
                case 'j':
                        run_jvmsim = 1;
                        break;
                case 'h':
                        usage();
                        exit(0);
                default:
                        handled = 0;
                        break;
        }
        return handled;
}

/* This is the defensive team. They're trying to block the offense */
void *thread_defense(void* arg)
{
	pthread_mutex_lock(&mutex);
	defense_count++;
	pthread_mutex_unlock(&mutex);

	/*keep the ball from being moved */
	while (1) {
		sched_yield(); /* let other defenders run */
	}
	return NULL;
}


/* This is the offensive team. They're trying to move the ball */
void *thread_offense(void* arg)
{
	pthread_mutex_lock(&mutex);
	offense_count++;
	pthread_mutex_unlock(&mutex);

	while (1) {
		the_ball++; /* move the ball ahead one yard */
		sched_yield(); /* let other offensive players run */
	}
	return NULL;
}

int referee(int game_length)
{
	struct timeval start, now;
	int final_ball;

	printf("Game On (%d seconds)!\n", game_length);

	gettimeofday(&start, NULL);
	now = start;
	the_ball = 0;

	/* Watch the game */
	while ((now.tv_sec - start.tv_sec) < game_length) {
		sleep(1);
		gettimeofday(&now, NULL);
	}
	/* Blow the whistle */
	printf("Game Over!\n");
        final_ball = the_ball;
	printf("Final ball position: %d\n", final_ball);
	return final_ball != 0;
}


void create_thread_(void*(*func)(void*), int prio)
{
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param param;

	param.sched_priority = sched_get_priority_min(SCHED_FIFO) + prio;

	pthread_attr_init(&attr);
	pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	if (pthread_create(&thread, &attr, func, (void *)0)) {
		perror("pthread_create failed");
	}

	pthread_attr_destroy(&attr);
}

int main(int argc, char* argv[])
{
	struct sched_param param;
	int players_per_team, game_length;
	int priority;
	int i;
	setup();

	rt_init("jh",parse_args,argc,argv);

	if (run_jvmsim) {
                printf("jvmsim enabled\n");
                jvmsim_init();  // Start the JVM simulation
	} else {
                printf("jvmsim disabled\n");
	}

	if (argc < 2 || argc > 3) {
		printf("Usage: %s players_per_team [game_length (seconds)]\n", argv[0]);
		players_per_team = sysconf(_SC_NPROCESSORS_ONLN);
		game_length = DEF_GAME_LENGTH;
		printf("Using default values: players_per_team=%d game_length=%d\n",
		       players_per_team, game_length);
	}

	else {
		players_per_team = atoi(argv[1]);
		if (argc == 3)
			game_length = atoi(argv[2]);
		else
			game_length = DEF_GAME_LENGTH;
	}

	/* We're the ref, so set our priority right */
	param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 80;
	sched_setscheduler(0, SCHED_FIFO, &param);

	/* Start the offense */
	priority = 15;
	printf("Starting %d offense threads at priority %d\n",
			players_per_team, priority);
	for (i = 0; i < players_per_team; i++)
		create_thread_(thread_offense, priority);
	while (offense_count < players_per_team)
		usleep(100);

	/* Start the defense */
	priority = 30;
	printf("Starting %d defense threads at priority %d\n",
			players_per_team, priority);
	for (i = 0; i < players_per_team; i++)
		create_thread_(thread_defense, priority);
	while (defense_count < players_per_team)
		usleep(100);

	/* Ok, everyone is on the field, bring out the ref */
	printf("Starting referee thread\n");
	return referee(game_length);
}

