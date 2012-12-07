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

int create_Result_file(void)
{
	int i, nbVal;
	double tabR[20000], Val_X, Val_Y;
	char *F_name, *F_namei1, *F_namei;
	int fp, fpi1, fpi;

	F_name = "pow_out.ref";
	F_namei = "pow_inp.ref";
	F_namei1 = "1pow_inp.ref";
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
			read(fpi, &Val_X, sizeof(double));
			read(fpi1, &Val_Y, sizeof(double));
			tabR[i] = pow(Val_X, Val_Y);
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
	double tabD[20000], tabD_pow[20000], Inc, Inc_pow;
	char *F_name, *F_name_pow;
	int fp, fp2;

	F_name = "pow_inp.ref";
	F_name_pow = "1pow_inp.ref";
	nbVal = 20000;

	Inc = exp(1);
	Inc_pow = exp(1) / 100;

	for (i = 0; i < nbVal; i++) {
		tabD_pow[nbVal - (i + 1)] = Inc_pow * i + Inc_pow;
		tabD[i] = (Inc * i) + Inc;
	}

	fp = open(F_name, O_RDWR | O_CREAT | O_TRUNC, 0777);
	fp2 = open(F_name_pow, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (!fp || !fp2) {
		printf("error opening file");
		close(fp);
		close(fp2);
		return -1;
	} else {
		for (i = 0; i < nbVal; i++) {
			write(fp, &tabD[i], sizeof(double));
			write(fp2, &tabD_pow[i], sizeof(double));
		}
		close(fp);
		close(fp2);
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
