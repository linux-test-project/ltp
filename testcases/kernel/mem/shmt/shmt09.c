/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 12/20/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

/*
 * NAME
 *	shmt09
 *
 * CALLS
 *	sbrk(2) shmctl(2) shmget(2)
 *
 * ALGORITHM
 * Create a shared memory segment and attach at the default address.
 * Increase the size of the data segment.
 * Attach at an address that is less than the break value: should FAIL.
 * decrease the size of the data segment.
 * Attach at 4K past the break value: should SUCCEED.
 * Sbrk() past the attached segment: should FAIL.
 *
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

#define K_1  1024

/** LTP Port **/
#include "test.h"
#include "usctest.h"

extern int errno;

char *TCID="shmt09";            /* Test program identifier.    */
int TST_TOTAL=4;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */
/**************/

int rm_shm(int);

int main()
{
	char	*c1=NULL, *c2=NULL, *c3=NULL;
	void	*vp;
	int	shmid;
	key_t	key;


	key = (key_t)getpid() ;

/*-----------------------------------------------------------*/


	if ((int)sbrk(16384) < 0 ) {
		perror("sbrk");
		tst_resm(TFAIL, 
		"Error: sbrk failed, errno = %d\n", errno) ;
		tst_exit() ;
	}

	if ((int)sbrk(-4097) < 0 ) {
		perror("sbrk");
		tst_resm(TFAIL, 
		"Error: sbrk failed, errno = %d\n", errno) ;
		tst_exit() ;
	}

	if ((shmid = shmget(key, 10*K_1, IPC_CREAT|0666)) < 0) {
		perror("shmget");
		tst_resm(TFAIL, 
		"Error: shmget Failed, shmid = %d, errno = %d\n", 
		shmid, errno) ;
		tst_exit() ;
	}

	c1 = (char *) shmat(shmid, (void *)0, 0);
	if (c1 == (char *)-1) {
		perror("shmat");
		tst_resm(TFAIL, 
		"Error: shmat Failed, shmid = %d, errno = %d\n", 
		shmid, errno) ;
		rm_shm(shmid) ;
		tst_exit() ;
	}

	tst_resm(TPASS,"sbrk, sbrk, shmget, shmat");	

/*--------------------------------------------------------*/


	if ((int)sbrk(32*K_1) < 0) {
		perror("sbrk");
		tst_resm(TFAIL, 
		"Error: sbrk failed, errno = %d\n", errno) ;
		rm_shm(shmid) ;
		tst_exit() ;
	}
	vp = (void *) ((char *)sbrk(0) - 2 * K_1);
	c2 = (char *) shmat(shmid, vp, 0);
	if (c2 != (char *)-1) {
		tst_resm(TFAIL,
		  "ERROR: shmat: succeeded!: shmid = %d, shmaddr = 0x%08x, "
		  "att_addr = 0x%08x\n",
		  shmid, c2, vp);
		rm_shm(shmid);
		tst_exit();
	}

	tst_resm(TPASS,"sbrk, shmat");

/*---------------------------------------------------------*/


	if ((int)sbrk(-16000) < 0) {
		perror("sbrk");
		tst_resm(TFAIL, 
		"Error: sbrk failed, errno = %d\n", errno) ;
		rm_shm(shmid) ;
		tst_exit() ;
	}

	vp = (void *) ((char *)sbrk(0) + 4 * K_1);
	c3 = (char *) shmat(shmid, vp, SHM_RND);
	if (c3 == (char *)-1) {
		perror("shmat1");
		tst_resm(TFAIL, 
		"Error: shmat Failed, shmid = %d, errno = %d\n", 
		shmid, errno) ;
		rm_shm(shmid) ;
		tst_exit() ;
	}

	tst_resm(TPASS,"sbrk, shmat");

/*--------------------------------------------------------*/


	if ((vp = sbrk(16000)) != (void *)-1) {
		tst_resm(TFAIL,
		  "Error: sbrk succeeded!  ret = 0x%08x, curbrk = 0x%08x, ",
		  vp, sbrk(0));
		rm_shm(shmid);
		tst_exit();
	}

	tst_resm(TPASS,"sbrk");

/*------------------------------------------------------*/
	
	rm_shm(shmid) ;
	tst_exit() ;

/*-----------------------------------------------------*/
	return(0);
}

int rm_shm(shmid)
int shmid ;
{
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
                perror("shmctl");
                tst_resm(TFAIL,
                "shmctl Failed to remove: shmid = %d, errno = %d\n",
                shmid, errno) ;
                tst_exit();
        }
        return(0);
}

