/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
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
 */

/******************************************************************************/
/*                                                                            */
/* Dec-03-2001  Created: Jacky Malcles & Jean Noel Cordenner                  */
/*              These tests are adapted from AIX float PVT tests.             */
/*                                                                            */
/******************************************************************************/
#include "tfloat.h"

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }
/*
 * allocates a buffer and read a file to it
 * input parameters:
 *	fname: name of the file to read
 *	data:  pointer where buffer addr. will be returned
 *
 * uses also external variable datadir to build file pathname
 *
 * returns:
 *	0 in case of failure
 *	# of bytes read elsewhere
 */
static size_t read_file(char *fname, void **data)
{
	struct stat bufstat;
	char path[PATH_MAX];
	size_t fsize;
	void *buffer;
	int fd;
	int maxretries = 1;

	(void)sprintf(path, "%s/%s", datadir, fname);

	errno = 0;

	while (stat(path, &bufstat)) {
		if (errno == ETIMEDOUT || errno == EINTR || errno == 0) {
			printf("Error stat'ing %s: %s\n",
			       path, strerror(errno));
			pthread_testcancel();
			/* retrying... */
			if (maxretries--)
				continue;
		}
		return (size_t) 0;
	}

	fsize = bufstat.st_size;
	if (!fsize) {
		errno = ENOENT;
		return (size_t) 0;
	}

	while ((buffer = malloc(fsize)) == NULL) {
		if (errno == EINTR || errno == 0) {
			printf("Error malloc'ing: %s\n", strerror(errno));
			pthread_testcancel();
			/* retrying... */
			if (maxretries--)
				continue;
		}
		return (size_t) 0;
	}

	while ((fd = open(path, O_RDONLY)) < 0) {
		if (errno == ETIMEDOUT || errno == EINTR || errno == 0) {
			printf("Error opening %s: %s\n", path, strerror(errno));
			pthread_testcancel();
			/* retrying... */
			if (maxretries--)
				continue;
		}
		SAFE_FREE(buffer);
		return (size_t) 0;
	}

	while (read(fd, buffer, fsize) != fsize) {
		if (errno == ETIMEDOUT || errno == EINTR || errno == 0) {
			printf("Error reading %s: %s\n", path, strerror(errno));
			pthread_testcancel();
			/* retrying... */
			if (lseek(fd, (off_t) 0, SEEK_SET) == (off_t) 0) {
				if (maxretries--)
					continue;
			}
		}
		(void)close(fd);
		SAFE_FREE(buffer);
		return (size_t) 0;
	}

	(void)close(fd);
	*data = buffer;
	return fsize;
}

/* this subroutine is used in compute_xxx functions to check results
   and record errors if appropriate */
static void check_error(TH_DATA * th_data, double e, double r, int index)
{
	double x;
	int pe, pr, px;
	static const char errtmplt[] =
	    "%s failed at index %d: OLD: %2.18e NEW: %2.18e DIFF: %2.18e\n";

	x = fabs(r - e);	/* diff expected/computed */

	if (x > EPS) {		/* error ? */
		/* compute exponent parts */
		(void)frexp(r, &pr);	/* for computed */
		(void)frexp(x, &px);	/* for difference */
		(void)frexp(e, &pe);	/* for dexected */

		if (abs(pe - px) < th_data->th_func.precision ||
		    abs(pr - px) < th_data->th_func.precision) {
			/* not a rounding error */
			++th_data->th_nerror;
			/* record first error only ! */
			if (th_data->th_result == 0) {
				sprintf(th_data->detail_data,
					errtmplt,
					th_data->th_func.fident,
					index, e, r, x);
				th_data->th_result = 1;
			}
		}
	}
}

/*
 * these functions handle the various cases of computation
 * they are called by pthread_code
 */

/* normal case: compares f(input data) to expected data */
static void compute_normal(TH_DATA * th_data, double *din, double *dex,
			   int index)
{
	double d, r, e;

	d = din[index];
	e = dex[index];
	r = (*(th_data->th_func.funct)) (d);

	check_error(th_data, e, r, index);
}

/* atan2 and hypot case: compares f(sin(input data),cos(input data))
   to expected data */
static void compute_atan2_hypot(TH_DATA * th_data, double *din, double *dex,
				int index)
{
	double d, r, e;

	d = din[index];
	e = dex[index];
	r = (*(th_data->th_func.funct)) (sin(d), cos(d));

	check_error(th_data, e, r, index);
}

/* modf case: compares integral and fractional parts to expected datas */
static void compute_modf(TH_DATA * th_data, double *din, double *dex,
			 double *dex2, int index)
{
	static const char errtmplt1[] =
	    "%s failed at index %d: OLD integral part: %f NEW: %f\n";
	double d, r, e;
	double tmp;

	d = din[index];
	e = dex[index];
	r = (*(th_data->th_func.funct)) (d, &tmp);

	if (tmp != dex2[index]) {	/* bad integral part! */
		++th_data->th_nerror;
		/* record first error only ! */
		if (th_data->th_result == 0) {
			sprintf(th_data->detail_data,
				errtmplt1,
				th_data->th_func.fident,
				index, dex2[index], tmp);
			th_data->th_result = 1;
		}
		return;
	}

	check_error(th_data, e, r, index);
}

/* fmod and pow case: compares f(input data, input data2) to expected data */
static void compute_fmod_pow(TH_DATA * th_data, double *din, double *dex,
			     double *dex2, int index)
{
	double d, r, e;

	d = din[index];
	e = dex[index];
	r = (*(th_data->th_func.funct)) (d, dex2[index]);

	check_error(th_data, e, r, index);
}

/* frexp case: compares mantissa and exponent to expected datas */
/* lgamma case: compares result and signgam to expected datas */
static void compute_frexp_lgamma(TH_DATA * th_data, double *din, double *dex,
				 int *dex2, int index)
{
	static const char errtmplt2[] =
	    "%s failed at index %d: OLD (exp. or sign): %d NEW: %d\n";
	double d, r, e;
	int tmp;
	static const char xinf[8] = "lgamma";

	d = din[index];
	e = dex[index];
	r = (*(th_data->th_func.funct)) (d, &tmp);

	if (strcmp(th_data->th_func.fident, xinf) != 0) {
		if (tmp != dex2[index]) {	/* bad exponent! */
			++th_data->th_nerror;
			/* record first error only ! */
			if (th_data->th_result == 0) {
				sprintf(th_data->detail_data,
					errtmplt2,
					th_data->th_func.fident,
					index, dex2[index], tmp);
				th_data->th_result = 1;
			}
			return;
		}
	}

	check_error(th_data, e, r, index);
}

/* ldexp case: compares f(input data, input data2) to expected data */
static void compute_ldexp(TH_DATA * th_data, double *din, double *dex,
			  int *din2, int index)
{
	double d, r, e;

	d = din[index];
	e = dex[index];
	r = (*(th_data->th_func.funct)) (d, din2[index]);

	check_error(th_data, e, r, index);
}

/*
 * Function which does the job, to be called as the
 * "start routine" parameter of pthread_create subroutine.
 * Uses the compute_xxx subroutines above.
 *
 * input parameters ("arg" parameter of pthread_create subroutine):
 *	pointer to a TH_DATA structure.
 *
 */
static void *thread_code(void *arg)
{
	TH_DATA *th_data = (TH_DATA *) arg;
	size_t fsize, fsize2, fsize3;
	double *din, *dex, *dex2 = NULL;
	int imax, index;

	fsize = read_file(th_data->th_func.din_fname, (void **)&din);
	if (fsize == (size_t) 0) {
		sprintf(th_data->detail_data,
			"FAIL: %s: reading %s, %s\n",
			th_data->th_func.fident,
			th_data->th_func.din_fname, strerror(errno));
		th_data->th_result = 1;
		SAFE_FREE(din);
		pthread_exit((void *)1);
	}
	fsize2 = read_file(th_data->th_func.dex_fname, (void **)&dex);
	if (fsize2 == (size_t) 0) {
		sprintf(th_data->detail_data,
			"FAIL: %s: reading %s, %s\n",
			th_data->th_func.fident,
			th_data->th_func.dex_fname, strerror(errno));
		th_data->th_result = 1;
		SAFE_FREE(din);
		SAFE_FREE(dex);
		pthread_exit((void *)1);
	}

	fsize3 = (size_t) 0;
	switch (th_data->th_func.code_funct) {
	case FUNC_MODF:
	case FUNC_FMOD:
	case FUNC_POW:
	case FUNC_FREXP:
	case FUNC_LDEXP:
	case FUNC_GAM:
		fsize3 = read_file(th_data->th_func.dex2_fname, (void **)&dex2);
		if (fsize3 == (size_t) 0) {
			sprintf(th_data->detail_data,
				"FAIL: %s: reading %s, %s\n",
				th_data->th_func.fident,
				th_data->th_func.dex2_fname, strerror(errno));
			th_data->th_result = 1;
			SAFE_FREE(din);
			SAFE_FREE(dex);
			pthread_exit((void *)1);
		}
	}

	switch (th_data->th_func.code_funct) {
	case FUNC_NORMAL:
	case FUNC_ATAN2:
	case FUNC_HYPOT:
		if (fsize2 != fsize)
			goto file_size_error;
		break;
	case FUNC_MODF:
	case FUNC_FMOD:
	case FUNC_POW:
		if (fsize2 != fsize || fsize3 != fsize)
			goto file_size_error;
		break;
	case FUNC_FREXP:
	case FUNC_LDEXP:
	case FUNC_GAM:
		if (fsize2 != fsize ||
		    (sizeof(double) / sizeof(int)) * fsize3 != fsize)
			goto file_size_error;
		break;
	default:
file_size_error:
		sprintf(th_data->detail_data,
			"FAIL: %s: file sizes don't match\n",
			th_data->th_func.fident);
		th_data->th_result = 1;
		SAFE_FREE(din);
		SAFE_FREE(dex);
		if (fsize3)
			SAFE_FREE(dex2);
		pthread_exit((void *)1);
	}

	imax = fsize / sizeof(double);

	while (th_data->th_nloop <= num_loops) {
		/* loop stopped by pthread_cancel */

		for (index = th_data->th_num; index < imax; index += num_threads) {	/* computation loop */
			switch (th_data->th_func.code_funct) {
			case FUNC_NORMAL:
				compute_normal(th_data, din, dex, index);
				break;
			case FUNC_ATAN2:
			case FUNC_HYPOT:
				compute_atan2_hypot(th_data, din, dex, index);
				break;
			case FUNC_MODF:
				compute_modf(th_data, din, dex, dex2, index);
				break;
			case FUNC_FMOD:
			case FUNC_POW:
				compute_fmod_pow(th_data,
						 din, dex, dex2, index);
				break;
			case FUNC_FREXP:
			case FUNC_GAM:
				compute_frexp_lgamma(th_data,
						     din, dex, (int *)dex2,
						     index);
				break;
			case FUNC_LDEXP:
				compute_ldexp(th_data,
					      din, dex, (int *)dex2, index);
				break;
			default:
				sprintf(th_data->detail_data,
					"FAIL: %s: unexpected function type\n",
					th_data->th_func.fident);
				th_data->th_result = 1;
				SAFE_FREE(din);
				SAFE_FREE(dex);
				if (fsize3)
					SAFE_FREE(dex2);
				pthread_exit((void *)1);
			}
			pthread_testcancel();
		}		/* end of computation loop */
		++th_data->th_nloop;
	}			/* end of loop */
	SAFE_FREE(din);
	SAFE_FREE(dex);
	if (fsize3)
		SAFE_FREE(dex2);
	pthread_exit(NULL);
}
