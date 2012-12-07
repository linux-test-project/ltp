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
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/signal.h>
#include <math.h>

/* **************************************
 *   create result file
 *
 *  the result is divided into 2 files
 * 1 double frationnal part of the input result of modf
 * 1 double which is the integral part of the input: tabRI
 *
 */
int create_Result_file(void)
{
	int i, nbVal;
	double tabR[20000], Val, Val1;
	char *F_name, *F_namei1, *F_namei;
	int fp, fpi1, fpi;

	F_name = "fmod_out.ref";
	F_namei = "fmod_inp.ref";
	F_namei1 = "1fmod_inp.ref";
	nbVal = 20000;

	fpi = open(F_namei, O_RDONLY, 0777);
	fpi1 = open(F_namei1, O_RDONLY, 0777);
	if (!fpi || !fpi1) {
		printf("error opening file");
		close(fpi);
		close(fpi1);
		return -1;
	} else {
		for (i = 0; i < nbVal; i++) {
			read(fpi, &Val, sizeof(double));
			read(fpi1, &Val1, sizeof(double));

			tabR[i] = fmod(Val, Val1);
		}
		close(fpi);
		close(fpi1);

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

int create_Data_file(void)
{
	int i, nbVal;
	double tabD[20000], tabD1[20000], Inc;
	char *F_name, *F_name1;
	int fp, fp1;

	F_name = "fmod_inp.ref";
	F_name1 = "1fmod_inp.ref";
	nbVal = 20000;

	Inc = exp(1) / 10;

	for (i = 0; i < nbVal; i++) {
		tabD[i] = log((Inc * i) + Inc);
		tabD1[i] = log(nbVal - (Inc * i) + Inc);
	}

	fp = open(F_name, O_RDWR | O_CREAT | O_TRUNC, 0777);
	fp1 = open(F_name1, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (!fp || !fp1) {
		printf("error opening file");
		close(fp);
		close(fp1);
		return -1;
	} else {
		for (i = 0; i < nbVal; i++) {
			write(fp, &tabD[i], sizeof(double));
			write(fp1, &tabD1[i], sizeof(double));
		}
		close(fp);
		close(fp1);
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

	return 0;
}
