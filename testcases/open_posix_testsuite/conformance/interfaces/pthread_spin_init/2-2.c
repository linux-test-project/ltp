/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 *	Test pthread_spin_init(pthread_spinlock_t *lock, int pshared)
 *
 * 	If the Thread Process-Shared Synchronization option is supported 
 * 	and the value of pshared is PTHREAD_PROCESS_PRIVATE, or if the option 
 * 	is not supported, the spin lock shall only be operated upon by threads created 
 * 	within the same process as the thread that initialized the spin lock. 
 *	If threads of different processed attempt to operation on such a spin
 *	lock, the behavior is undefined.
 *	
 * NOTE: This case will always PASS 
 *
 * steps:
 *	1. Create a piece of shared memory object, create a spin lock 'spinlock' and
 *	   set the PTHREAD_PROCESS_PRIVATE attribute.
 *	2. Parent map the shared memory to its memory space, put 'spinlock' into it;
 *	3. Parent get the spin lock; 	
 *	4. Fork to create child
 *	5. Child map the shared memory to its memory space;
 *	6. Child call pthread_spin_trylock()
 *	7. Main unlock
 *	8. Child call pthread_spin_trylock()
 */


#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "posixtest.h"

struct shmstruct{
	pthread_spinlock_t spinlock;
	int data;
} *spinlock_data;

int main()
{
	
	int pshared;

	/* Make sure there is process-shared capability. */ 
	#ifdef PTHREAD_PROCESS_PRIVATE
	pshared = PTHREAD_PROCESS_PRIVATE;
	#else
 	pshared = -1;
	#endif
	
	char shm_name[] = "tmp_pthread_spinlock_init";
	int shm_fd;
	int pid;
	int rc;

	/* Create shared object */
	shm_unlink(shm_name);
	shm_fd = shm_open(shm_name, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if(shm_fd == -1)
	{
		perror("Error at shm_open()");
		return PTS_UNRESOLVED;
	}
     
        if(ftruncate(shm_fd, sizeof(struct shmstruct)) != 0) {
                perror("Error at ftruncate()");
                shm_unlink(shm_name);
                return PTS_UNRESOLVED;
        }
	
	/* Map the shared memory object to parent's memory */
	spinlock_data = mmap(NULL, sizeof(struct shmstruct), PROT_READ|PROT_WRITE, 
				MAP_SHARED, shm_fd, 0);

	if(spinlock_data == MAP_FAILED)
	{
		perror("Error at first mmap()");
                shm_unlink(shm_name);
		return PTS_UNRESOLVED;
	}
	
	if((pthread_spin_init(&(spinlock_data->spinlock), pshared)) != 0)
	{
		printf("Test FAILED: Error at pthread_rwlock_init()\n");
		return PTS_FAIL;
	}
	
	printf("main: attempt spin lock\n");	
	if((pthread_spin_lock(&(spinlock_data->spinlock))) != 0)
	{
		printf("Error at pthread_spin_lock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired spin lock\n");

	/* Initialized spinlock data */
	spinlock_data->data = 0;
	
	pid = fork();
	if(pid == -1)
	{
		perror("Error at fork()");
		return PTS_UNRESOLVED;
	}
	else if(pid > 0)
	{
		/* Parent */
		/* wait until child writes to spinlock data */
		while(spinlock_data->data != 1)
			sleep(1);
		
		printf("main: unlock spin lock\n");	
		if(pthread_spin_unlock(&(spinlock_data->spinlock)) != 0)
		{
			printf("main: error at pthread_spin_unlock()\n");
			return PTS_UNRESOLVED;
		}
	
		/* Tell child that parent unlocked the spin lock */
		spinlock_data->data = 2;
	
		/* Wait until child ends */
		wait(NULL);
		
		if((shm_unlink(shm_name)) != 0)
		{
			perror("Error at shm_unlink()");
			return PTS_UNRESOLVED;
		}	
		
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	else
	{
		/* Child */
		/* Map the shared object to child's memory */
		spinlock_data = mmap(NULL, sizeof(struct shmstruct), PROT_READ|PROT_WRITE, 
				MAP_SHARED, shm_fd, 0);

		if(spinlock_data == MAP_FAILED)
		{
			perror("child : Error at mmap()");
			return PTS_UNRESOLVED;
		}
		
		printf("child: attempt spin lock\n");
		rc = pthread_spin_trylock(&(spinlock_data->spinlock));
		if( rc != EBUSY)
			printf("child: get return code %d, %s\n", rc, strerror(rc));
		else
			printf("child: correctly got EBUSY\n");	
		
		/* Tell parent it can unlock now */
		spinlock_data->data = 1;
		
		while(spinlock_data->data != 2)
			sleep(1);
		
		printf("child: attempt spin lock\n");
		rc = pthread_spin_trylock(&(spinlock_data->spinlock));
		if(rc == 0)
			printf("child: acquired spin lock\n");
		else
			printf("child: get return code %d, %s\n", rc, strerror(rc));

		printf("child: unlock spin lock\n");	
		if(pthread_spin_unlock(&(spinlock_data->spinlock)) != 0)
		{
			printf("Child: error at pthread_spin_unlock()\n");
			return PTS_UNRESOLVED;
		}

		if(pthread_spin_destroy(&(spinlock_data->spinlock)) != 0)
		{
			printf("Child: error at pthread_spin_destroy()\n");
			return PTS_UNRESOLVED;
		}
	}
}
