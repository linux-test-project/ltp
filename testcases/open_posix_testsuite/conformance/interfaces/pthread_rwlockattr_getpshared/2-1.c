/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 *	The process-shared attribute shall be set to PTHREAD_PROCESS_SHARED 
 *	to permit a read-write lock to be operated upon by any thread 
 *	that has access to the memory where the read-write lock is allocated, 
 *	even if the read-write lock is allocated in memory that is shared by
 *	multiple processes. 
 * 
 * steps:
 *	1. Create a piece of shared memory object, create a read write lock 'rwlock' and
 *	   set the PTHREAD_PROCESS_SHARED attribute.
 *	2. Parent map the shared memory to its memory space, put 'rwlock' into it;
 *	3. Parent get read lock; 	
 *	4. Fork to create child, parent wait until child call pthread_rwlock_wrlock()
 *	5. Child map the shared memory to its memory space;
 *	6. Child call pthread_rwlock_trywrlock(), should fail with EBUSY;
 *	7. Child call pthread_rwlock_wrlock(), should block;   
 *	8. Parent unlock 'rwlock'
 *	9. Child should get the write lock.
 */


#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "posixtest.h"

struct shmstruct{
	pthread_rwlock_t rwl;
	int data;
} *rwlock_data;

int main()
{
	
	/* Make sure there is process-shared capability. */ 
	#ifndef PTHREAD_PROCESS_SHARED
	  fprintf(stderr,"process-shared attribute is not available for testing\n");
	  return PTS_UNSUPPORTED;	
	#endif

	pthread_rwlockattr_t rwla;
	int pshared = PTHREAD_PROCESS_SHARED;
	
	char shm_name[] = "tmp_pthread_rwlock_getpshared";
	int shm_fd;
	int pid;
	
	/* Initialize a rwlock attributes object */
	if(pthread_rwlockattr_init(&rwla) != 0)
	{
		printf("Error at pthread_rwlockattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlockattr_setpshared(&rwla, pshared) != 0)
	{
		printf("Error at pthread_rwlockattr_setpshared()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlockattr_getpshared(&rwla, &pshared) != 0)
	{
		printf("Test FAILED: Error at pthread_rwlockattr_getpshared()\n");
		return PTS_FAIL;
	}
	
	if(pshared != PTHREAD_PROCESS_SHARED)
	{
		printf("Test FAILED: Got error shared attribute value %d\n", pshared);
		return PTS_FAIL;
	}
	
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
	
	/* Map the shared memory object to my memory */
	rwlock_data = mmap(NULL, sizeof(struct shmstruct), PROT_READ|PROT_WRITE, 
				MAP_SHARED, shm_fd, 0);

	if(rwlock_data == MAP_FAILED)
	{
		perror("Error at first mmap()");
                shm_unlink(shm_name);
		return PTS_UNRESOLVED;
	}
	
	if((pthread_rwlock_init(&(rwlock_data->rwl), &rwla)) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	if((pthread_rwlockattr_destroy(&rwla)) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("Parent getting read lock.\n");	
	if((pthread_rwlock_rdlock(&(rwlock_data->rwl))) != 0)
	{
		printf("Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("Parent got read lock.\n");
	rwlock_data->data = 0;
	
	pid = fork();
	if(pid == -1)
	{
		perror("Error at fork()");
		return PTS_UNRESOLVED;
	}
	else if(pid > 0)
	{
		/* Parent */
		/* wait until child do wrlock */
		while(rwlock_data->data == 0)
		{
			sleep(1);
		}
		
		printf("Parent unlocking.\n");	
		if(pthread_rwlock_unlock(&(rwlock_data->rwl)) != 0)
		{
			printf("Parent: error at pthread_rwlock_unlock()\n");
			return PTS_FAIL;
		}
		printf("Parent unlocked.\n");
		
		/* Wait for child to end */
		wait(NULL);
		
		if((shm_unlink(shm_name)) != 0)
		{
			perror("Error at shm_unlink()");
			return PTS_UNRESOLVED;
		}	
		
		if(rwlock_data->data == -1)
		{
			printf("Test FAILED: child did not block on write lock\n");
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}
	else
	{
		/* Child */
		/* Map the shared object to child's memory */
		rwlock_data = mmap(NULL, sizeof(struct shmstruct), PROT_READ|PROT_WRITE, 
				MAP_SHARED, shm_fd, 0);

		if(rwlock_data == MAP_FAILED)
		{
			perror("Error at first mmap()");
			return PTS_UNRESOLVED;
		}
		
		printf("Child tries to get write lock, should get EBUSY.\n");
		if((pthread_rwlock_trywrlock(&(rwlock_data->rwl))) != EBUSY)
		{
			printf("Test FAILED: Child expects EBUSY\n");
			return PTS_FAIL;
		} 
		printf("Child got EBUSY.\n");	
		
		printf("Child do wrlock.\n");	

		/* Tell parent it can unlock now */
		rwlock_data->data = 1;
		
		/* Should block until parent unlock */
		if((pthread_rwlock_wrlock(&(rwlock_data->rwl))) != 0)
		{
			printf("Child:pthread_rwlock_wrlock() error\n");
			printf("Test FAILED: Error while write lock the shared rwlock\n");
			rwlock_data->data = -1;
			return PTS_FAIL;
		}
		printf("Child got wrlock.\n");
		if((pthread_rwlock_unlock(&(rwlock_data->rwl))) != 0)
		{
			printf("Child:pthread_rwlock_unlock() error\n");
			printf("Error while write unlock the shared rwlock\n");
			rwlock_data->data = -1;
			return PTS_FAIL;
		}
	}
}
