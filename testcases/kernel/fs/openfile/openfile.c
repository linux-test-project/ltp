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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * FILE        : openfile.c
 * DESCRIPTION : Create files and open simultaneously
 * HISTORY:
 *   03/21/2001 Paul Larson (plars@us.ibm.com)
 *     -Ported
 *   11/01/2001 Mnaoj Iyer (manjo@austin.ibm.com)
 *     - Modified. 
 *	 added #inclide <unistd.h>
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>


#define MAXFILES        32768
#define MAXTHREADS      10 


/* Control Structure */
struct cb {
	pthread_mutex_t m;
	pthread_cond_t  init_cv;
	pthread_cond_t  thr_cv;
	int		thr_sleeping;
} c;


/* Global Variables */
int     numthreads=10, numfiles=10;
int     debug=0;
char *  filename="FILETOOPEN";


/* Procedures */
void *threads(void* thread_id);



/* **************************************************************************
   *                              MAIN PROCEDURE                            *
   ************************************************************************** */
	
int main(int argc, char *argv[])
{
	int 	   i,opt,badopts=0;
        FILE 	   *fd;
	pthread_t  th_id;
	char       msg[80]="";
        extern char *optarg;

        while((opt=getopt(argc, argv, "df:t:h")) != EOF) {
          switch((char) opt) {
            case 'd':
              debug=1;
              break;
            case 'f':
              numfiles=atoi(optarg);
              if(numfiles <= 0)
                badopts=1;
              break;
            case 't':
              numthreads=atoi(optarg);
              if(numthreads <= 0)
                badopts=1;
              break;
            case 'h':
            default:
              printf("Usage: openfile [-d] -f FILES -t THREADS\n");
       	      _exit(1);
          }
        }
        if(badopts) {
          printf("Usage: openfile [-d] -f FILES -t THREADS\n");
       	  _exit(1);
        }

    	/* Check if numthreads is less than MAXFILES */
    	if ( numfiles > MAXFILES ){
          	sprintf(msg,"%s\nCannot use %d files", msg, numfiles);
          	sprintf(msg,"%s, used %d files instead\n", msg, MAXFILES);
          	numfiles = MAXFILES;
    	}

    	/* Check if numthreads is less than MAXTHREADS */
    	if ( numthreads > MAXTHREADS ){
          	sprintf(msg,"%s\nCannot use %d threads", msg, numthreads);
          	sprintf(msg,"%s, used %d threads instead\n", msg, MAXTHREADS);
          	numthreads = MAXTHREADS;
	}

	/* Create files */
	if ((fd=fopen(filename,"w")) == NULL){	
		perror ("FAIL - Could not create file");
		_exit(1);
	}

	/* Initialize thread control variables, lock & condition */
	pthread_mutex_init(&c.m, NULL);
	pthread_cond_init(&c.init_cv, NULL);
	pthread_cond_init(&c.thr_cv, NULL);
	c.thr_sleeping = 0;

	/* Grab mutex lock */
	if (pthread_mutex_lock(&c.m)){
		perror("FAIL - failed to grab mutex lock");
		fclose(fd);
		unlink(filename);
		_exit(1);
	}

	printf("Creating Reading Threads\n");

	/* Create threads */
	for (i=0; i<numthreads; i++)
		if (pthread_create(&th_id, (pthread_attr_t *)NULL, threads,
				(void *)(uintptr_t)i)) {
			perror("FAIL - failed creating a pthread; increase limits");
			fclose(fd);
			unlink(filename);
			_exit(1);
		}

	/* Sleep until all threads are created */
	while (c.thr_sleeping != numthreads)
		if (pthread_cond_wait(&c.init_cv, &c.m)){
			perror("FAIL - error while waiting for reading threads");
			fclose(fd);
			unlink(filename);
			_exit(1);
		}

	/* Wake up all threads */
	if (pthread_cond_broadcast(&c.thr_cv)){
		perror("FAIL - failed trying to wake up reading threads");
		fclose(fd);
		unlink(filename);
		_exit(1);
	}
	/* Release mutex lock */
	if (pthread_mutex_unlock(&c.m)) {
		perror("FAIL - failed to release mutex lock");
		fclose(fd);
		unlink(filename);
		_exit(1);
	}

        printf("PASS - Threads are done reading\n");

    	printf("%s", msg);
	fclose(fd);
	unlink(filename);
	_exit(0);
}



/* **************************************************************************
   *				OTHER PROCEDURES			    *
   ************************************************************************** */

/* threads: Each thread opens the files specified */
void * threads(void* thread_id_)
{
  int thread_id=(uintptr_t)thread_id_;
    	char  errmsg[80];
        FILE  *fd;
	int   i;

    	/* Open files */
    	for (i=0; i<numfiles; i++) {
                if(debug)
       		  printf("Thread  %d : Opening file number %d \n", thread_id, i);
       		if ((fd = fopen(filename,"rw")) == NULL) {
          		sprintf(errmsg,"FAIL - Couldn't open file #%d",i);
          		perror(errmsg);
			unlink(filename);
			pthread_exit((void*)1);
       		}
    	}

	/* Grab mutex lock */
	if (pthread_mutex_lock(&c.m)) {
		perror("FAIL - failed to grab mutex lock");
       		fclose(fd);
		unlink(filename);
		pthread_exit((void*)1);
	}
	
	/* Check if you should wake up main thread */
        if (++c.thr_sleeping == numthreads)
                if (pthread_cond_signal(&c.init_cv)){
			perror("FAIL - failed to signal main thread");
       			fclose(fd);
			unlink(filename);
			pthread_exit((void*)1);
		}

	/* Sleep until woken up */
        if (pthread_cond_wait(&c.thr_cv, &c.m)){
		perror("FAIL - failed to wake up correctly");
       		fclose(fd);
		unlink(filename);
		pthread_exit((void*)1);
	}

	/* Release mutex lock */
        if (pthread_mutex_unlock(&c.m)){
		perror("FAIL - failed to release mutex lock");
       		fclose(fd);
		unlink(filename);
		pthread_exit((void*)1);
	}

	/* Close file and exit */
       	fclose(fd);
	unlink(filename);
        pthread_exit((void*)0);
}


