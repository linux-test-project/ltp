/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
* $Id: main.c,v 1.11 2009/02/26 12:14:53 subrata_modak Exp $
*
*/
#include <stdio.h>
#ifdef WINDOWS
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include "getopt.h"
#else
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "globals.h"
#include "main.h"
#include "usage.h"
#include "sfunc.h"
#include "parse.h"
#include "childmain.h"
#include "threading.h"
#include "dump.h"
#include "timer.h"
#include "stats.h"
#include "signals.h"

/* global */
child_args_t cleanArgs;
test_env_t cleanEnv;
char hostname[HOSTNAME_SIZE];	/* global system hostname */

void linear_read_write_test(test_ll_t * test)
{
	OFF_T *pVal1 = (OFF_T *) test->env->shared_mem;
	int i;

	if (test->args->flags & CLD_FLG_W) {
		test->env->bContinue = TRUE;
		*(pVal1 + OFF_WLBA) = test->args->start_lba;
		test->args->test_state = DIRCT_INC(test->args->test_state);
		test->env->lastAction.oper = WRITER;
		test->args->test_state = SET_OPER_W(test->args->test_state);
		test->args->test_state = SET_wFST_TIME(test->args->test_state);
//              srand(test->args->seed);        /* reseed so we can re create the same random transfers */
		memset(test->env->action_list, 0,
		       sizeof(action_t) * test->args->t_kids);
		test->env->action_list_entry = 0;
		test->env->wcount = 0;
		test->env->rcount = 0;
		if (test->args->flags & CLD_FLG_CYC)
			if (test->args->cycles == 0) {
				pMsg(INFO, test->args,
				     "Starting write pass, cycle %lu\n",
				     (unsigned long)test->env->pass_count);
			} else {
				pMsg(INFO, test->args,
				     "Starting write pass, cycle %lu of %lu\n",
				     (unsigned long)test->env->pass_count,
				     test->args->cycles);
		} else {
			pMsg(INFO, test->args, "Starting write pass\n");
		}
		CreateTestChild(ChildTimer, test);
		for (i = 0; i < test->args->t_kids; i++) {
			CreateTestChild(ChildMain, test);
		}
		/* Wait for the writers to finish */
		cleanUpTestChildren(test);
	}

	/* If the write test failed don't start the read test */
	if (!(TST_STS(test->args->test_state))) {
		return;
	}

	if (test->args->flags & CLD_FLG_R) {
		test->env->bContinue = TRUE;
		*(pVal1 + OFF_RLBA) = test->args->start_lba;
		test->args->test_state = DIRCT_INC(test->args->test_state);
		test->env->lastAction.oper = READER;
		test->args->test_state = SET_OPER_R(test->args->test_state);
		test->args->test_state = SET_rFST_TIME(test->args->test_state);
//              srand(test->args->seed);        /* reseed so we can re create the same random transfers */
		memset(test->env->action_list, 0,
		       sizeof(action_t) * test->args->t_kids);
		test->env->action_list_entry = 0;
		test->env->wcount = 0;
		test->env->rcount = 0;
		if (test->args->flags & CLD_FLG_CYC)
			if (test->args->cycles == 0) {
				pMsg(INFO, test->args,
				     "Starting read pass, cycle %lu\n",
				     (unsigned long)test->env->pass_count);
			} else {
				pMsg(INFO, test->args,
				     "Starting read pass, cycle %lu of %lu\n",
				     (unsigned long)test->env->pass_count,
				     test->args->cycles);
		} else {
			pMsg(INFO, test->args, "Starting read pass\n");
		}
		CreateTestChild(ChildTimer, test);
		for (i = 0; i < test->args->t_kids; i++) {
			CreateTestChild(ChildMain, test);
		}
		/* Wait for the readers to finish */
		cleanUpTestChildren(test);
	}
}

unsigned long init_data(test_ll_t * test, unsigned char **data_buffer_unaligned)
{
	int i;
	OFF_T *pVal1;

	unsigned long data_buffer_size;

#ifdef WINDOWS
	if (CreateMutex(NULL, FALSE, "gbl") == NULL) {
		pMsg(ERR, test->args,
		     "Failed to create semaphore, error = %u\n",
		     GetLastError());
		return (GetLastError());
	}
	if ((test->env->mutexs.MutexACTION =
	     CreateMutex(NULL, FALSE, NULL)) == NULL) {
		pMsg(ERR, test->args,
		     "Failed to create semaphore, error = %u\n",
		     GetLastError());
		return (GetLastError());
	}
	if ((test->env->mutexs.MutexIO =
	     CreateMutex(NULL, FALSE, NULL)) == NULL) {
		pMsg(ERR, test->args,
		     "Failed to create semaphore, error = %u\n",
		     GetLastError());
		return (GetLastError());
	}
#else

	mutexs_t mutexs =
	    { PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };
	test->env->mutexs = mutexs;

#endif

	if (test->args->seed == 0)
		test->args->seed = test->args->pid;
	srand(test->args->seed);

	/* create bitmap to hold write/read context: each bit is an LBA */
	/* the stuff before BMP_OFFSET is the data for child/thread shared context */
	test->env->bmp_siz =
	    (((((size_t) test->args->vsiz)) / 8) ==
	     0) ? 1 : ((((size_t) test->args->vsiz)) / 8);
	if ((test->args->vsiz / 8) != 0)
		test->env->bmp_siz += 1;	/* account for rounding error */

	/* We use that same data buffer for static data, so alloc here. */
	data_buffer_size = ((test->args->htrsiz * BLK_SIZE) * 2);
	if ((*data_buffer_unaligned =
	     (unsigned char *)ALLOC(data_buffer_size + ALIGNSIZE)) == NULL) {
		pMsg(ERR, test->args,
		     "Failed to allocate static data buffer memory.\n");
		return (-1);
	}
	/* create list to hold lbas currently be written */
	if ((test->env->action_list =
	     (action_t *) ALLOC(sizeof(action_t) * test->args->t_kids)) ==
	    NULL) {
		pMsg(ERR, test->args,
		     "Failed to allocate static data buffer memory.\n");
		return (-1);
	}

	test->env->data_buffer =
	    (unsigned char *)BUFALIGN(*data_buffer_unaligned);

	if ((test->env->shared_mem =
	     (void *)ALLOC(test->env->bmp_siz + BMP_OFFSET)) == NULL) {
		pMsg(ERR, test->args, "Failed to allocate bitmap memory\n");
		return (-1);
	}

	memset(test->env->shared_mem, 0, test->env->bmp_siz + BMP_OFFSET);
	memset(test->env->data_buffer, 0, data_buffer_size);
	memset(test->env->action_list, 0,
	       sizeof(action_t) * test->args->t_kids);
	test->env->action_list_entry = 0;

	pVal1 = (OFF_T *) test->env->shared_mem;
	*(pVal1 + OFF_WLBA) = test->args->start_lba;
	*(pVal1 + OFF_RLBA) = test->args->start_lba;
	test->args->test_state = SET_STS_PASS(test->args->test_state);
	test->args->test_state = SET_wFST_TIME(test->args->test_state);
	test->args->test_state = SET_rFST_TIME(test->args->test_state);
	test->args->test_state = DIRCT_INC(test->args->test_state);
	if (test->args->flags & CLD_FLG_W) {
		test->env->lastAction.oper = WRITER;
		test->args->test_state = SET_OPER_W(test->args->test_state);
	} else {
		test->env->lastAction.oper = READER;
		test->args->test_state = SET_OPER_R(test->args->test_state);
	}

	/* prefill the data buffer with data for compares and writes */
	switch (test->args->flags & CLD_FLG_PTYPS) {
	case CLD_FLG_FPTYPE:
		for (i = 0; i < sizeof(test->args->pattern); i++) {
			if ((test->args->
			     pattern & (((OFF_T) 0xff) <<
					(((sizeof(test->args->pattern) - 1) -
					  i) * 8))) != 0)
				break;
		}
		/* special case for pattern = 0 */
		if (i == sizeof(test->args->pattern))
			i = 0;
		fill_buffer(test->env->data_buffer, data_buffer_size,
			    &test->args->pattern,
			    sizeof(test->args->pattern) - i, CLD_FLG_FPTYPE);
		break;
	case CLD_FLG_RPTYPE:
		fill_buffer(test->env->data_buffer, data_buffer_size, NULL, 0,
			    CLD_FLG_RPTYPE);
		break;
	case CLD_FLG_CPTYPE:
		fill_buffer(test->env->data_buffer, data_buffer_size, 0, 0,
			    CLD_FLG_CPTYPE);
	case CLD_FLG_LPTYPE:
		break;
	default:
		pMsg(WARN, test->args, "Unknown fill pattern\n");
		return (-1);
	}

	return 0;
}

#ifdef WINDOWS
DWORD WINAPI threadedMain(test_ll_t * test)
#else
void *threadedMain(void *vtest)
#endif
{
#ifndef WINDOWS
	test_ll_t *test = (test_ll_t *) vtest;
#endif

	OFF_T *pVal1;
	unsigned char *data_buffer_unaligned = NULL;
	unsigned long ulRV;
	int i;
	unsigned char *sharedMem;

	extern unsigned short glb_run;
	extern int signal_action;

	test->args->pid = GETPID();

	init_gbl_data(test->env);

	if (make_assumptions(test->args) < 0) {
		TEXIT((uintptr_t) GETLASTERROR());
	}
	if (check_conclusions(test->args) < 0) {
		TEXIT((uintptr_t) GETLASTERROR());
	}
	if (test->args->flags & CLD_FLG_DUMP) {
		/*
		 * All we are doing is dumping filespec data to STDOUT, so
		 * we will do this here and be done.
		 */
		do_dump(test->args);
		TEXIT((uintptr_t) GETLASTERROR());
	} else {
		ulRV = init_data(test, &data_buffer_unaligned);
		if (ulRV != 0) {
			TEXIT(ulRV);
		}
		pVal1 = (OFF_T *) test->env->shared_mem;
	}

	pMsg(START, test->args, "Start args: %s\n", test->args->argstr);

	/*
	 * This loop takes care of passes
	 */
	do {
		test->env->pass_count++;
		test->env->start_time = time(NULL);
		if (test->args->flags & CLD_FLG_RPTYPE) {	/* force random data to be different each cycle */
			fill_buffer(test->env->data_buffer,
				    ((test->args->htrsiz * BLK_SIZE) * 2), NULL,
				    0, CLD_FLG_RPTYPE);
		}
		sharedMem = test->env->shared_mem;
		memset(sharedMem + BMP_OFFSET, 0, test->env->bmp_siz);
		if ((test->args->flags & CLD_FLG_LINEAR)
		    && !(test->args->flags & CLD_FLG_NTRLVD)) {
			linear_read_write_test(test);
		} else {
			/* we only reset the end time if not running a linear read / write test */
			test->env->end_time =
			    test->env->start_time + test->args->run_time;
			test->env->bContinue = TRUE;
			*(pVal1 + OFF_WLBA) = test->args->start_lba;
			test->args->test_state =
			    DIRCT_INC(test->args->test_state);
			test->args->test_state =
			    SET_wFST_TIME(test->args->test_state);
			test->args->test_state =
			    SET_rFST_TIME(test->args->test_state);
			if (test->args->flags & CLD_FLG_W) {
				test->env->lastAction.oper = WRITER;
				test->args->test_state =
				    SET_OPER_W(test->args->test_state);
			} else {
				test->env->lastAction.oper = READER;
				test->args->test_state =
				    SET_OPER_R(test->args->test_state);
			}
			memset(test->env->action_list, 0,
			       sizeof(action_t) * test->args->t_kids);
			test->env->action_list_entry = 0;
			test->env->wcount = 0;
			test->env->rcount = 0;

			if (test->args->flags & CLD_FLG_CYC)
				if (test->args->cycles == 0) {
					pMsg(INFO, test->args,
					     "Starting pass %lu\n",
					     (unsigned long)test->env->
					     pass_count);
				} else {
					pMsg(INFO, test->args,
					     "Starting pass %lu of %lu\n",
					     (unsigned long)test->env->
					     pass_count, test->args->cycles);
			} else {
				pMsg(INFO, test->args, "Starting pass\n");
			}

			CreateTestChild(ChildTimer, test);
			for (i = 0; i < test->args->t_kids; i++) {
				CreateTestChild(ChildMain, test);
			}
			/* Wait for the children to finish */
			cleanUpTestChildren(test);
		}

		update_cyc_stats(test->env);
		if ((test->args->flags & CLD_FLG_CYC)
		    && (test->args->flags & CLD_FLG_PCYC)) {
			print_stats(test->args, test->env, CYCLE);
		}
		update_gbl_stats(test->env);

		if (signal_action & SIGNAL_STOP) {
			break;
		}		/* user request to stop */
		if ((glb_run == 0)) {
			break;
		}
		/* global request to stop */
		if (!(test->args->flags & CLD_FLG_CYC)) {
			break;	/* leave, unless cycle testing */
		} else {
			if ((test->args->cycles > 0)
			    && (test->env->pass_count >= test->args->cycles)) {
				break;	/* leave, cycle testing complete */
			}
		}
	} while (TST_STS(test->args->test_state));
	print_stats(test->args, test->env, TOTAL);

	FREE(data_buffer_unaligned);
	FREE(test->env->shared_mem);
#ifdef WINDOWS
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "gbl"));
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "data"));
#endif

	if (TST_STS(test->args->test_state)) {
		if (signal_action & SIGNAL_STOP) {
			pMsg(END, test->args,
			     "User Interrupt: Test Done (Passed)\n");
		} else {
			pMsg(END, test->args, "Test Done (Passed)\n");
		}
	} else {
		if (signal_action & SIGNAL_STOP) {
			pMsg(END, test->args,
			     "User Interrupt: Test Done (Failed)\n");
		} else {
			pMsg(END, test->args, "Test Done (Failed)\n");
		}
	}
	TEXIT((uintptr_t) GETLASTERROR());
}

/*
 * Creates a new test structure and adds it to the list of
 * test structures already available.  Allocate all memory
 * needed by the new test.
 *
 * Returns the newly created test structure
 */
test_ll_t *getNewTest(test_ll_t * testList)
{
	test_ll_t *pNewTest;

	if ((pNewTest = (test_ll_t *) ALLOC(sizeof(test_ll_t))) == NULL) {
		pMsg(ERR, &cleanArgs,
		     "%d : Could not allocate memory for new test.\n",
		     GETLASTERROR());
		return NULL;
	}

	memset(pNewTest, 0, sizeof(test_ll_t));

	if ((pNewTest->args =
	     (child_args_t *) ALLOC(sizeof(child_args_t))) == NULL) {
		pMsg(ERR, &cleanArgs,
		     "%d : Could not allocate memory for new test.\n",
		     GETLASTERROR());
		FREE(pNewTest);
		return NULL;
	}
	if ((pNewTest->env = (test_env_t *) ALLOC(sizeof(test_env_t))) == NULL) {
		pMsg(ERR, &cleanArgs,
		     "%d : Could not allocate memory for new test.\n",
		     GETLASTERROR());
		FREE(pNewTest->args);
		FREE(pNewTest);
		return NULL;
	}
	memcpy(pNewTest->args, &cleanArgs, sizeof(child_args_t));
	memcpy(pNewTest->env, &cleanEnv, sizeof(test_env_t));

	pNewTest->next = testList;
	testList = pNewTest;
	return pNewTest;
}

test_ll_t *run()
{
	test_ll_t *newTest = NULL, *lastTest = NULL;

	if (cleanArgs.flags & CLD_FLG_FSLIST) {
		char *filespec = cleanArgs.device;
		char *aFilespec = NULL;
		FILE *file = NULL;

		if ((aFilespec = (char *)ALLOC(80)) == NULL) {
			pMsg(ERR, &cleanArgs,
			     "Could not allocate memory to read file");
			return newTest;
		}

		file = fopen(filespec, "r");
		if (file == NULL) {
			pMsg(ERR,
			     &cleanArgs,
			     "%s is not a regular file, could not be opened for reading, or was not found.",
			     filespec);
			FREE(aFilespec);

			return newTest;
		}

		while (!feof(file)) {
			memset(aFilespec, 0, 80);
			fscanf(file, "%79s", aFilespec);
			if (aFilespec[0] != 0) {	/* if we read something useful */
				lastTest = newTest;
				newTest = getNewTest(lastTest);
				if (newTest != lastTest) {
					memset(newTest->args->device, 0,
					       DEV_NAME_LEN);
					strncpy(newTest->args->device,
						aFilespec, strlen(aFilespec));
					createChild(threadedMain, newTest);
				} else {
					newTest = lastTest;
					break;
				}
			}
		}

		fclose(file);
		FREE(aFilespec);
	} else {
		newTest = getNewTest(newTest);
		if (newTest != NULL) {
			createChild(threadedMain, newTest);
		}
	}

	return newTest;
}

int main(int argc, char **argv)
{
	extern time_t global_start_time;
	extern unsigned long glb_flags;	/* global flags GLB_FLG_xxx */
	int i;

#ifdef WINDOWS
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		pMsg(WARN, &cleanArgs,
		     "Windows setup of Winsock failed, can't retrieve host name, continuing");
	}
#endif

	setup_sig_mask();

	memset(hostname, 0, HOSTNAME_SIZE);
	gethostname(hostname, HOSTNAME_SIZE);

	setbuf(stdout, NULL);

	glb_flags = 0;
	global_start_time = time(NULL);

	strncpy(cleanArgs.device, "No filespec", strlen("No filespec"));
	cleanArgs.stop_lba = -1;
	cleanArgs.stop_blk = -1;
	cleanArgs.ioTimeout = DEFAULT_IO_TIMEOUT;
	cleanArgs.flags |= CLD_FLG_ALLDIE;
	cleanArgs.flags |= CLD_FLG_ERR_REREAD;
	cleanArgs.flags |= CLD_FLG_LBA_SYNC;

	for (i = 1; i < argc - 1; i++) {
		strncat(cleanArgs.argstr, argv[i],
			(MAX_ARG_LEN - 1) - strlen(cleanArgs.argstr));
		strncat(cleanArgs.argstr, " ",
			(MAX_ARG_LEN - 1) - strlen(cleanArgs.argstr));
	}

	if (fill_cld_args(argc, argv, &cleanArgs) < 0)
		exit(1);

	cleanUp(run());

#ifdef WINDOWS
	WSACleanup();
#endif

	return 0;
}
