/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 */
#include <config.h>
#include <errno.h>
#include <unistd.h>
#include <openhpi.h>  

		 
#ifdef HAVE_THREAD_SAFE
/* multi-threading support, use Posix mutex for data access */
/* initialize mutex used for data locking */
static pthread_mutex_t data_access_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static int will_block = 0;

void data_access_lock(void) 
{
	if (pthread_mutex_trylock(&data_access_mutex) == EBUSY) {
		pthread_mutex_lock(&data_access_mutex);	
	        will_block++;
	}
}

void data_access_unlock(void)
{
        pthread_mutex_unlock(&data_access_mutex);    
}

int data_access_block_times(void) 
{
        return(will_block);
}
#else 
void data_access_lock(void) {}
void data_access_unlock(void){} 
int data_access_block_times(void){ return(0);}
#endif/*HAVE_THREAD_SAFE*/
