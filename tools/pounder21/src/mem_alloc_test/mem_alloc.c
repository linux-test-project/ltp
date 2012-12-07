/* ************************************************************************************
 *	mem_alloc.c
 *	@description : This program will consume memory using sbrk() to a size where
 *		       there is about COMMITED_AS KB left in free{swap+ram}.
 *		       The program realized that a process can consume so much memory,
 *		       space, so it will fork more child to consume as much as memory
 *		       possible, aiming for final free{swap+ram} < COMMITED_AS.
 *		       EXEPTION: If overcommit_momory is set, the program will only
 *			         consume as much as momory as oom-killer allows, and
 *				 will exit when then limit reached even the
 *				 free{swap+ram} not < COMMITTED_AS KB.
 *	@author	     : Sarunya Jimenez (sjimene@us.ibm.com)
 * ********************************************************************************** */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <errno.h>

/////////////////////////// GLOBAL STATIC VAIRABLE FOR SIGNAL HANDLER /////////////////
static volatile sig_atomic_t sigflag;	// set nonzero by sig handler
static sigset_t newmask, oldmask, zeromask;
////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////// GLOBAL DEFINES ////////////////////////////////////////
#define KB_VALUE        1024	// value in bytes -> 1024 bytes
#define COMMITTED_AS  102400	// value in KB    -> 102400 KB -> 100MB
#define MALLOC_SIZE   0x10000	// = 64KB... for each sbrk(MALLOC_SIZE) in
					// malloc_data()
					// MUST ALWAYS BE POSSITIVE VALUE
#define PAGE_SIZE     0x400	// = 1024 KB
/////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////// GLOBAL VARIABLES ////////////////////////////////////////
long sbrk_num;			// global sbrk_num to keep track of # of times sbrk() get called
char *start_addr;		// heap @before a process allocate memory - get updated in eat_mem()
char *end_addr;			// heap @after a process allocate memory  - get updated in alloc_data()
			// and dealloc_data()
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////// ERROR HANDLING PRINT FUNCTIONS //////////////////////////
/* ========================================================================================
 *	Print linux error message, will exit the current process.
 * ======================================================================================== */
void unix_error(char *msg)
{
	printf("LINUX ERROR: %s: %s\n", msg, strerror(errno));
	exit(0);
}

/* ========================================================================================
 *	Print functionality-error message for user process, will not exit the current process.
 * ======================================================================================== */
void user_error(char *msg)
{
	printf("APPLICATION ERROR: %s\n", msg);
}

/////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////// SIGNAL HANDLING FUNCTIONS ///////////////////////////////////////
/* =====================================================================================
 *	One Signal Handler for SIGUSR1 and SIGUSR2.
 * ===================================================================================== */
static void sig_usr(int signo)	// signal hanlder for SIGUSR1 and SIGUSR2
{
	sigflag = 1;
}

/* ========================================================================================
 *	SET UP signal handler before TELL_PARENT(), WAIT_PARENT(), TELL_CHILD(), WAIT_CHILD().
 *	- This function must be called before fork() and TELL/WAIT_PARENT/CHILD() functions.
 * ======================================================================================== */
void TELL_WAIT(void)
{
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		unix_error("signal (SIGUSR1) FAILED");
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		unix_error("signal (SIGUSR2) FAILED");

	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		unix_error("signal (SIG_BLOCK) FAILED");
}

/* ========================================================================================
 *	TELL parent that we are done: used in child process.
 *	- This function must be called after TELL_WAIT() setup the SIGUSR1 & SIGUSR2 propery.
 *	- INPUT: parent process ID; can be obtained through getppid().
 * ======================================================================================== */
void TELL_PARENT(pid_t pid)
{
	kill(pid, SIGUSR2);	// send signal SIGUSR2 to pid process
}

/* ========================================================================================
 *	TELL child that we are done: used in parent process.
 *	- This function must be called after TELL_WAIT() setup the SIGUSR1 & SIGUSR2 propery.
 *	- INPUT: child process ID; can be obtained through pid = fork() where pid > 0.
 * ======================================================================================== */
void TELL_CHILD(pid_t pid)
{
	kill(pid, SIGUSR1);	// send signal SIGUSR1 to pid process
}

/* ========================================================================================
 *	WAIT for parent: used in child process.
 *	- This function must be called after TELL_WAIT() setup the SIGUSR1 & SIGUSR2 propery.
 * ======================================================================================== */
void WAIT_PARENT(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);	// wait for child
	sigflag = 0;

	// reset signal mask to original value
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		unix_error("signal (SIG_SETMASK) FAILED");
}

/* ========================================================================================
 *	WAIT for child: used in parent process.
 *	- This function must be called after TELL_WAIT() setup the SIGUSR1 & SIGUSR2 propery.
 * ======================================================================================== */
void WAIT_CHILD(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);	// wait for parent
	sigflag = 0;

	// reset signal mask to original value
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		unix_error("signal (SIG_SETMASK) FAILED");
}

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////// MEMORY ALLOCATION FUNCTIONS /////////////////////////
/* =====================================================================================
 *	SET sbrk_num @start of each process to count # of sbrk() calls within that process.
 *	- INPUT: input number for globak sbrk_num to be set to.
 * ===================================================================================== */
void set_sbrk_num(int in)
{
	sbrk_num = in;
}

/* ========================================================================================
 *	PRINT system information; e.g. free {ram, swap}, total {ram, swap}.
 * ======================================================================================== */
void print_sysinfo(void)
{
	struct sysinfo si;
	sysinfo(&si);

	printf
	    ("freeram (%luKB),  freeswap (%luKB), totalram (%luKB), totalswap (%luKB)\n",
	     (si.freeram / KB_VALUE) * si.mem_unit,
	     (si.freeswap / KB_VALUE) * si.mem_unit,
	     (si.totalram / KB_VALUE) * si.mem_unit,
	     (si.totalswap / KB_VALUE) * si.mem_unit);
}

/* ========================================================================================
 *	CALCULATE freeswap space.
 *	- OUTPUT: Return size of free swap space in KB.
 * ======================================================================================== */
long unsigned freeswap(void)
{
	struct sysinfo si;
	sysinfo(&si);

	return ((si.freeswap / KB_VALUE) * si.mem_unit);
}

/* ========================================================================================
 *	CALCULATE freeram space.
 *	- OUTPUT: Return size of free ram space in KB.
 * ======================================================================================== */
long unsigned freeram(void)
{
	struct sysinfo si;
	sysinfo(&si);

	return ((si.freeram / KB_VALUE) * si.mem_unit);
}

/* ========================================================================================
 *	ALLOCATE data using sbrk(incr).
 *  	- Global sbrk_num will be updated for each time calling sbrk() to increase heap size.
 *	- OUTPUT: Return 1 if success,
 *			 0 if failed, and will decrement the heap space for future library calls
 * ======================================================================================== */
int malloc_data(void)
{
	int return_value = 0;	// default return = true;
	intptr_t incr = MALLOC_SIZE;	// 64KB
	char *src = NULL;	// to hold addr return from sbrk(incr)
	long i;			// loop counter

	src = sbrk(incr);

	if (((void *)src == (void *)-1) && (errno == ENOMEM)) {	// error handling
		src = sbrk(-(2 * incr));	// freeing some space for later library calls
		sbrk_num -= 2;
		end_addr = src + (-(2 * incr));	// update end of heap
	} else {		// sucess case
		// must write to data, write once for each 1KB
		for (i = 0x0; i < incr; i += PAGE_SIZE)
			src[i] = '*';
		++sbrk_num;	// update global sbrk() call counter when success
		return_value = 1;	// update return value to true
		end_addr = src + incr;	// update end of heap
	}

	return return_value;
}

/* ========================================================================================
 *	DEALLOCATE data using sbrk(-incr).
 *  	- Global sbrk_num will be updated for each time calling sbrk() to decrease heap size.
 *	- OUTPUT: Return 1 if success,
 *			 0 if failed.
 * ======================================================================================== */
int dealloc_data(void)
{
	int return_value = 0;	// default return = true
	intptr_t incr = MALLOC_SIZE;	// 64KB
	char *src = NULL;	// to hold adrr return from sbrk(incr)
	long i;			// loop counter
	long old_sbrk_num = sbrk_num;	// save old sbrk_num counter, because sbrk_num will be updated

	for (i = 0; i < old_sbrk_num; ++i) {
		src = sbrk(-incr);

		// error handling: Fatal Fail
		if (((void *)src == (void *)-1) && (errno == ENOMEM))
			goto OUT;	// error

		--sbrk_num;	// update # of sbrk() call
		end_addr = src + (-incr);	// update end of heap
	}
	return_value = 1;	// update return value to true

OUT:
	return return_value;
}

/* ========================================================================================
 *	Write to the memory because of Copy-On-Write behavior from LINUX kernel.
 *	IDEA: Because fork() is implemented through Copy-On-Write. This technique
 *	      delay/prevent the copy of data, child & parent share memory, and their
 *	      duplication of the address of child & parent are shared read-only. For parent
 *	      & child to have its very own separate space, both must write to their own data.
 *	      So this function will deal with the write for the child process created
 *	      by fork().
 *	OUTPUT: Return 1 if success,
 *		       0 if failed.
 * ======================================================================================== */
int handle_COW(void)
{
	int return_value = 0;	// default return = true
	intptr_t incr = MALLOC_SIZE;	// 64KB
	char *src = NULL;	// to hold adrr return from sbrk(incr)
	char *i;		// loop counter

	// error handling: Make sure the start_addr is not NULL
	if (start_addr == NULL) {
		user_error("start_addr from parent is not initialized");
		goto OUT;
	}
	// error handling: Make sure the end_addr is not NULL
	if (end_addr == NULL) {
		user_error("end_addr from parent is not initialized");
		goto OUT;
	}
	// Writing to heap
	if (start_addr < end_addr) {	// Heap grows up to higher address
		for (i = start_addr; i < end_addr; i += PAGE_SIZE) {
			if ((freeswap() + freeram()) < COMMITTED_AS)
				goto OUT;
			*i = 'u';
		}
		return_value = 1;
	} else if (start_addr > end_addr) {	// Heap grows down to lower address
		for (i = end_addr; i > start_addr; i -= PAGE_SIZE) {
			if ((freeswap() + freeram()) < COMMITTED_AS)
				goto OUT;
			*i = 'd';
		}
		return_value = 1;
	} else;			// Heap doesn't grows

OUT:
	return return_value;
}

/* ========================================================================================
 *	EAT lots and lots of memory...
 *	- If a process can eat all of the free resouces
 *	  specified, that process will exit the program.
 * ======================================================================================== */
void eat_mem(void)
{
	// saving the current heap pointer befoer start to allocate more memory
	start_addr = NULL;
	end_addr = NULL;
	start_addr = sbrk(0);

	// eating memory
	while ((freeswap() + freeram()) > COMMITTED_AS) {
		if (!malloc_data())
			return;
	}

	print_sysinfo();
	exit(0);
}

/* ========================================================================================
 *	EAT lots and lots of memory...If a process can eat all of the free resouces
 *	specified, that process will exit the program
 * ======================================================================================== */
void eat_mem_no_exit(void)
{
	// saving the current heap pointer befoer start to allocate more memory
	start_addr = NULL;
	end_addr = NULL;
	start_addr = sbrk(0);

	// eating memory
	while ((freeswap() + freeram()) > COMMITTED_AS) {
		if (!malloc_data())
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////// MAIN PROGRAM ////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	pid_t pid;		// used for fork()
	print_sysinfo();	// sytem resouces before start allocation
	set_sbrk_num(0);	// at start of process, ensure sbrk_num is set
	eat_mem();

	// @beyound this point -> 1 process can't consume all memory so it must fork a child to consume more
	// memory
START:
	pid = fork();
	pid = pid < 0 ? -1 : pid;

	switch (pid) {
	case -1:
		if (!dealloc_data())
			unix_error
			    ("SBRK(-incr) FROM DEALLOC_DATA() FAILED. FATAL!!!");
		goto LAST_CONDITION;

	case 0:
		if (!handle_COW()) {	// Re-touch child pages
			print_sysinfo();	// FINAL RESULT, LAST RESOURCES
			exit(0);	// child can't allocate no more, DONE!!!
		}
		goto START;

	default:
		if (waitpid(-1, NULL, 0) != pid)	// Parent Waiting
			unix_error("WAIT_PID FAILED. FATAL!!!");
		exit(0);
	}

LAST_CONDITION:
	TELL_WAIT();		// set up parent/child signal handler
	pid = fork();
	pid = pid < 0 ? -1 : pid;

	switch (pid) {
	case -1:
		unix_error("FORK FAILED.");

	case 0:
		eat_mem_no_exit();
		WAIT_PARENT();
		print_sysinfo();	// FINAL RESULT, LAST RESOUCES
		TELL_PARENT(getppid());
		exit(0);

	default:
		eat_mem_no_exit();
		TELL_CHILD(pid);
		WAIT_CHILD();
		exit(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
