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
#include 	<float.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>
#include        <limits.h>
#include        <unistd.h>
#include        <fcntl.h>
#include        <errno.h>
#include        <sys/signal.h>
#include        <math.h>

/******************************************************************
 *
 *	genldexp
 *
 * generate input and output file  for the ldexp function
 * double x multiplied by 2 raised to the power y
 *
 *
 */

static int create_Result_file(void)
{

	int i, nbVal, tabInpi[20000];
	double tabR[20000], tabInpd[20000];
	char *F_name;
	char *F_namini;
	char *F_namind;
	int fp, fpi, fpd;

	F_name = "ldexp_out.ref";
	F_namini = "ildexp_inp.ref";
	F_namind = "ldexp_inp.ref";
	nbVal = 20000;

	fpi = open(F_namini, O_RDONLY, 0777);
	fpd = open(F_namind, O_RDONLY, 0777);

	if (!fpi || !fpd) {
		printf("error opening file");
		close(fpi);
		close(fpd);
		return -1;
	} else {
		for (i = 0; i < nbVal; i++) {
			read(fpi, &(tabInpi[i]), sizeof(int));
			read(fpd, &(tabInpd[i]), sizeof(double));
			tabR[i] = ldexp(tabInpd[i], tabInpi[i]);
		}
		close(fpi);
		close(fpd);

		fp = open(F_name, O_RDWR | O_CREAT | O_TRUNC, 0777);
		if (!fp) {
			printf("error opening file");
			close(fp);
			return -1;
		} else {
			for (i = 0; i < nbVal; i++) {
				write(fp, &tabR[i], sizeof(double));
			}

			close(fp);
			return 0;
		}
	}
}

/*********************************************************************
 *
 *	create input data file
 *
 *	the format of the data is double x   int y
 */

static int create_Data_file(void)
{
	int i, nbVal;
	double tabDD[20000], tabDI[20000], Inc;
	char *F_named, *F_namei;
	int fp, fpi;

	F_named = "ldexp_inp.ref";
	F_namei = "ildexp_inp.ref";
	nbVal = 20000;

	Inc = exp(1) / 10;

	for (i = 0; i < (nbVal); i++) {
		tabDD[i] = (Inc * i) + Inc;
		tabDI[i] = nbVal - i;
	}

	fp = open(F_named, O_RDWR | O_CREAT | O_TRUNC, 0777);
	fpi = open(F_namei, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (!fp || !fpi) {
		printf("error opening file");
		close(fp);
		close(fpi);
		return -1;
	} else {
		for (i = 0; i < nbVal; i++) {
			write(fp, &tabDD[i], sizeof(double));
			write(fpi, &tabDI[i], sizeof(int));
		}
		close(fp);
		close(fpi);
		return 0;
	}
}

int main(int argc, char *argv[])
{

	if (argc > 1) {
		switch (atoi(argv[1])) {
		case 1:
			if (create_Data_file() == 0)
				printf("Data file created\n");
			else
				printf("problem during %s data file creation\n",
				       argv[0]);
			break;

		case 2:
			if (create_Result_file() == 0)
				printf("Result file created\n");
			else
				printf
				    ("problem during %s result file creation\n",
				     argv[0]);
			break;
		default:
			printf("Bad arglist code for: '%s'\n", argv[0]);
			return -1;
			break;
		}
	} else {
		if (create_Data_file() != 0)
			printf("problem during %s data file creation\n",
			       argv[0]);
		if (create_Result_file() != 0)
			printf("problem during %s result file creation\n",
			       argv[0]);
	}

	return (0);

}
