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
 *       libstats.h
 *
 * DESCRIPTION
 *      Some basic statistical analysis convenience tools.
 *
 *
 * USAGE:
 *      To be included in test cases
 *
 * AUTHOR
 *        Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Oct-17: Initial version by Darren Hart
 *
 * TODO: the save routine for gnuplot plotting should be more modular...
 *
 *****************************************************************************/

#ifndef LIBSTATS_H
#define LIBSTATS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))

typedef struct stats_record {
	long x;
	long y;
} stats_record_t;

typedef struct stats_container {
	long size;
	long index;
	stats_record_t *records;
} stats_container_t;

enum stats_sort_method {
	ASCENDING_ON_X,
	ASCENDING_ON_Y,
	DESCENDING_ON_X,
	DESCENDING_ON_Y
};

typedef struct stats_quantiles {
	int nines;
	long *quantiles;
} stats_quantiles_t;

extern int save_stats;

/* function prototypes */

/* stats_container_init - allocate memory for a new container
 * size: number of records to allocate
 * data: stats_container_t destination pointer
 */
int stats_container_init(stats_container_t *data, long size);

/* stats_container_resize - resize a container
 * data: container to resize
 * size: new number of records
 */
int stats_container_resize(stats_container_t *data, long size);

/* stats_container_free - free the records array
 * data: stats_container_t to free records
 */
int stats_container_free(stats_container_t *data);

/* stats_sort - sort a container according to method
 * data: stats_container_t to sort
 * method: which field and which order to sort
 */
int stats_sort(stats_container_t *data, enum stats_sort_method method);

/* stats_stddev - return the standard deviation of the y values in data
 * data: stats_container_t data with y values for use in the calculation
 */
float stats_stddev(stats_container_t *data);

/* stats_avg - return the average (mean) of the y values in data
 * data: stats_container_t data with y values for use in the calculation
 */
float stats_avg(stats_container_t *data);

/* stats_min - return the minimum of the y values in data
 * data: stats_container_t data with y values for use in the calculation
 */
long stats_min(stats_container_t *data);

/* stats_max - return the maximum of the y values in data
 * data: stats_container_t data with y values for use in the calculation
 */
long stats_max(stats_container_t *data);

/* stats_container_init - allocate memory for new quantiles
 * nines: int number of nines in most inclusive quantile
 * quantiles: stats_quantiles_t destination pointer
 */
int stats_quantiles_init(stats_quantiles_t *quantiles, int nines);

/* stats_quantiles_free - free the quantiles array
 * data: stats_quantiles_t to free
 */
int stats_quantiles_free(stats_quantiles_t *quantiles);

/* stats_quantiles_calc - calculate the quantiles of the supplied container
 * data: stats_container_t data with y values for use in the calculation
 * quantiles: stats_quantiles_t structure for storing the results
 */
int stats_quantiles_calc(stats_container_t *data, stats_quantiles_t *quantiles);

/* stats_quantiles_print - print the quantiles stored in quantiles
 * quantiles: stats_quantiles_t structure to print
 */
void stats_quantiles_print(stats_quantiles_t *quantiles);

/* stats_hist - calculate a histogram with hist->size divisions from data
 * hist: the destination of the histogram data
 * data: the source from which to calculate the histogram
 */
int stats_hist(stats_container_t *hist, stats_container_t *data);

/* stats_hist_print - print out a histogram in human readable format
 * hist: the stats_container containing the histogram data
 */
void stats_hist_print(stats_container_t *hist);

/* stats_container_save - save the x,y data to a file and create a gnuplot
 * runnable script.
 * filename: the filename to save the data as without an extension. A .dat
 * and a .plt file will be created.
 * title: the title of the graph
 * labelx: the x-axis label
 * labely: the y-axis label
 * mode: "points" "lines" "steps" etc, see gnuplot help for plotting types
 */
int stats_container_save(char *filename, char *title, char *labelx, char *labely, stats_container_t *data, char *mode);

/* stats_container_append - appends stats_record_t to data
 * data: stats_container_t structure for holding the records list, index of
 *       min and max elements in records list and the sum
 * rec: stats_record_t to be appended to the records list in data
 * Returns the index of the appended record on success and -1 on error
 */
int stats_container_append(stats_container_t *data, stats_record_t rec);
#endif /* LIBSTAT_H */
