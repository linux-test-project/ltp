/* @BULL_COPYRIGHT@ */
/*
 * @(#)09  1.2  testcase/bfloat_MP/src/tfloat.h, pvt_kernel, pv_kernel 12/27/96 13:50:17
 *
 * Copyright (C) Bull S.A. 1996
 * Level 1,5 Years Bull Confidential and Proprietary Information
 */

#ifndef _TFLOAT_H
#define _TFLOAT_H
#define TRUE    (1)

#include <pthread.h> /* pthread.h header file must be the first included file */
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
/*extern char *sys_errlist[ ];*/

#ifdef __MATH__
/* this is to force use of math libraries (generates a warning with xlC 3.1.1)*/
#undef __MATH__
#endif
#include <math.h>

#define DETAIL_DATA_SIZE        256
#define FNAME_SIZE              64

#define FUNC_NORMAL            0
#define FUNC_ATAN2             1
#define FUNC_HYPOT             2
#define FUNC_MODF              3
#define FUNC_FMOD              4
#define FUNC_POW               5
#define FUNC_FREXP             6
#define FUNC_LDEXP             7
#define FUNC_GAM               8

extern void * thread_code(void *);

/* global variables, constants or initialized by main() */
extern const double EPS; /* 0.1e-300 */
extern int true, num_threads;

/*
 * TH_DATA structures
 * the main thread allocates and initializes one of these structures for
 * each launched thread. Its address is given to the thread.
 *
 *      th_num          thread # (range: 0 ... num_threads-1)
 * these 3 fields are used to get thread results. init. value = 0.
 *      th_result       result (0 = GOOD)
 *      th_nerror	number of errors found
 *      th_nloop	number of loops completed
 *      detail_data	when th_result!=0, contains error message
 *
 * the TH_FUNC structure contains all the data needed to execute the tests.
 *         code_funct	function type
 *         precision	int. value used to distinguish between rounding
 *      		errors and real ones (relative difference between
 *      		expected and read value should be less than
 *      		2 ** precision)
 *         funct        function pointer
 *         fident       function id. (string) for error messages
 *         din_fname    data file name (input data)
 *         dex_fname    data file name (expected data)
 *         dex2_fname   data file name (input or expected, optionnel)
 */
typedef struct {
        int code_funct;
	int precision;
        double (*funct)();
        char fident[16];
        char din_fname[FNAME_SIZE];
        char dex_fname[FNAME_SIZE];
        char dex2_fname[FNAME_SIZE];
} TH_FUNC;

typedef struct {
        int th_num;
        int th_result;
        int th_nerror;
        int th_nloop;
        char detail_data[DETAIL_DATA_SIZE];
	TH_FUNC th_func;
} TH_DATA;

extern const TH_FUNC th_func[];

#endif /* ifndef _TFLOAT_H */
