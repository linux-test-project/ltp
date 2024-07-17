/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006, 2008
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
 *
 * NAME
 *	   libstats.c
 *
 * DESCRIPTION
 *	  Some basic statistical analysis convenience tools.
 *
 *
 * USAGE:
 *	  To be included in test cases
 *
 * AUTHOR
 *		Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *	  2006-Oct-17: Initial version by Darren Hart
 *	  2009-Jul-22: Addition of stats_container_append function by Kiran Prakash
 *
 * TODO: the save routine for gnuplot plotting should be more modular...
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "libstats.h"
#include "librttest.h"

int save_stats = 0;

/* static helper functions */
static int stats_record_compare(const void *a, const void *b)
{
	int ret = 0;
	stats_record_t *rec_a = (stats_record_t *) a;
	stats_record_t *rec_b = (stats_record_t *) b;
	if (rec_a->y < rec_b->y)
		ret = -1;
	else if (rec_a->y > rec_b->y)
		ret = 1;
	return ret;
}

/* function implementations */
int stats_container_init(stats_container_t * data, long size)
{
	data->size = size;
	data->index = -1;
	data->records = calloc(size, sizeof(stats_record_t));
	if (!data->records)
		return -1;
	return 0;
}

int stats_container_append(stats_container_t * data, stats_record_t rec)
{
	int myindex = ++data->index;
	if (myindex >= data->size) {
		debug(DBG_ERR, "Number of elements cannot be more than %ld\n",
		      data->size);
		data->index--;
		return -1;
	}
	data->records[myindex] = rec;
	return myindex;
}

int stats_container_resize(stats_container_t * data, long size)
{
	stats_record_t *newrecords =
	    realloc(data->records, size * sizeof(stats_record_t));
	if (!newrecords)
		return -1;
	data->records = newrecords;
	if (data->size < size)
		memset(data->records + data->size, 0, size - data->size);
	data->size = size;
	return 0;
}

int stats_container_free(stats_container_t * data)
{
	free(data->records);
	return 0;
}

int stats_sort(stats_container_t * data, enum stats_sort_method method)
{
	/* method not implemented, always ascending on y atm */
	qsort(data->records, data->index + 1, sizeof(stats_record_t),
	      stats_record_compare);
	return 0;
}

float stats_stddev(stats_container_t * data)
{
	long i;
	float sd, avg, sum, delta;
	long n;

	sd = 0.0;
	n = data->index + 1;
	sum = 0.0;

	/* calculate the mean */
	for (i = 0; i < n; i++) {
		sum += data->records[i].y;
	}
	avg = sum / (float)n;

	/* calculate the standard deviation */
	sum = 0.0;
	for (i = 0; i < n; i++) {
		delta = (data->records[i].y - avg);
		sum += delta * delta;
	}
	sum /= n;

	sd = sqrt(sum);

	return sd;
}

float stats_avg(stats_container_t * data)
{
	long i;
	float avg, sum;
	long n;

	n = data->index + 1;
	sum = 0.0;

	/* calculate the mean */
	for (i = 0; i < n; i++) {
		sum += data->records[i].y;
	}
	avg = sum / (float)n;

	return avg;
}

long stats_min(stats_container_t * data)
{
	long i;
	long min;
	long n;

	n = data->index + 1;

	/* calculate the mean */
	min = data->records[0].y;
	for (i = 1; i < n; i++) {
		if (data->records[i].y < min) {
			min = data->records[i].y;
		}
	}

	return min;
}

long stats_max(stats_container_t * data)
{
	long i;
	long max;
	long n;

	n = data->index + 1;

	/* calculate the mean */
	max = data->records[0].y;
	for (i = 1; i < n; i++) {
		if (data->records[i].y > max) {
			max = data->records[i].y;
		}
	}

	return max;
}

int stats_quantiles_init(stats_quantiles_t * quantiles, int nines)
{
	if (nines < 2) {
		return -1;
	}
	quantiles->nines = nines;
	/* allocate space for quantiles, starting with 0.99 (two nines) */
	quantiles->quantiles = calloc(sizeof(long), (nines - 1));
	if (!quantiles->quantiles) {
		return -1;
	}
	return 0;
}

int stats_quantiles_free(stats_quantiles_t * quantiles)
{
	free(quantiles->quantiles);
	return 0;
}

int stats_quantiles_calc(stats_container_t * data,
			 stats_quantiles_t * quantiles)
{
	int i;
	int size;
	int index;

	// check for sufficient data size of accurate calculation
	if (data->index < 0 ||
	    (data->index + 1) < (long)exp10(quantiles->nines)) {
		return -1;
	}

	size = data->index + 1;
	stats_sort(data, ASCENDING_ON_Y);

	for (i = 2; i <= quantiles->nines; i++) {
		index = size - size / exp10(i);
		quantiles->quantiles[i - 2] = data->records[index].y;
	}
	return 0;
}

void stats_quantiles_print(stats_quantiles_t * quantiles)
{
	int i;
	int fraction = 0;
	for (i = 0; i <= quantiles->nines - 2; i++) {
		if (i > 0)
			fraction += 9 * exp10(i - 1);
		printf("99.%d%% < %ld\n", fraction, quantiles->quantiles[i]);
	}
}

int stats_hist(stats_container_t * hist, stats_container_t * data)
{
	int i;
	int ret;
	long min, max, width;
	long y, b;

	ret = 0;

	if (hist->size <= 0 || data->index < 0) {
		return -1;
	}

	/* calculate the range of dataset */
	min = max = data->records[0].y;
	for (i = 0; i <= data->index; i++) {
		y = data->records[i].y;
		if (y > max)
			max = y;
		if (y < min)
			min = y;
	}

	/* define the bucket ranges */
	width = MAX((max - min) / hist->size, 1);
	hist->records[0].x = min;
	for (i = 1; i < (hist->size); i++) {
		hist->records[i].x = min + i * width;
	}

	/* fill in the counts */
	for (i = 0; i <= data->index; i++) {
		y = data->records[i].y;
		b = MIN((y - min) / width, hist->size - 1);
		hist->records[b].y++;
	}

	return 0;
}

void stats_hist_print(stats_container_t * hist)
{
	long i, x;
	for (i = 0; i < hist->size; i++) {
		x = hist->records[i].x;
		if (i < hist->size - 1)
			printf("[%ld,%ld) = %ld\n", x,
			       hist->records[i + 1].x, hist->records[i].y);
		else
			printf("[%ld,-) = %ld\n", x, hist->records[i].y);
	}
}

int stats_container_save(char *filename, char *title, char *xlabel,
			 char *ylabel, stats_container_t * data, char *mode)
{
	int i;
	int minx = 0, maxx = 0, miny = 0, maxy = 0;
	FILE *dat_fd;
	FILE *plt_fd;
	char *datfile;
	char *pltfile;
	stats_record_t *rec;

	if (!save_stats)
		return 0;

	/* generate the filenames */
	if (asprintf(&datfile, "%s.dat", filename) == -1) {
		fprintf(stderr,
			"Failed to allocate string for data filename\n");
		return -1;
	}
	if (asprintf(&pltfile, "%s.plt", filename) == -1) {
		fprintf(stderr,
			"Failed to allocate string for plot filename\n");
		return -1;
	}

	/* generate the data file */
	if ((dat_fd = fopen(datfile, "w")) == NULL) {
		perror("Failed to open dat file");
		return -1;
	} else {
		minx = maxx = data->records[0].x;
		miny = maxy = data->records[0].y;
		for (i = 0; i <= data->index; i++) {
			rec = &data->records[i];
			minx = MIN(minx, rec->x);
			maxx = MAX(maxx, rec->x);
			miny = MIN(miny, rec->y);
			maxy = MAX(maxy, rec->y);
			fprintf(dat_fd, "%ld %ld\n", rec->x, rec->y);
		}
		fclose(dat_fd);
	}

	/* generate the plt file */
	if (!(plt_fd = fopen(pltfile, "w"))) {
		perror("Failed to open plt file");
		return -1;
	} else {
		fprintf(plt_fd, "set terminal png\n");
		fprintf(plt_fd, "set output \"%s.png\"\n", pltfile);
		fprintf(plt_fd, "set title \"%s\"\n", title);
		fprintf(plt_fd, "set xlabel \"%s\"\n", xlabel);
		fprintf(plt_fd, "set ylabel \"%s\"\n", ylabel);
		fprintf(plt_fd, "plot [0:%d] [0:%d] \"%s\" with %s\n",
			maxx + 1, maxy + 1, datfile, mode);
		fclose(plt_fd);
	}

	return 0;
}
