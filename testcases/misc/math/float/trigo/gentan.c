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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#define M_PIl	3.1415926535897932384626433832795029L

int create_Result_file()
{

	int i, nbVal;
	double	tabRtan[20000], Inc;
	char *F_name;
	int fp;

	F_name = "rtan";
	nbVal = 20000;

	Inc = (2*M_PIl)/nbVal;  /* condering a period of 2 pi rad */

	for (i=0; i<nbVal; i++)
	{
		if ((Inc*i) != (M_PIl/2))
			tabRtan[i] = tan (Inc * i);
		else
			tabRtan[i] = tan (0);
	}

	fp = open(F_name,O_RDWR|O_CREAT|O_TRUNC,0777);
        if (!fp)
        {
            	printf("error opening file");
		close(fp);
		return -1;
	}
	else
	{
		for (i = 0; i<nbVal; i++)
		{
			write(fp,&tabRtan[i],sizeof(double));
		}

		close(fp);
		return 0;
	}
}

int create_Data_file()
{
	int i, nbVal;
	double	tabDtan[20000], Inc;
	char *F_name;
	int fp;

	F_name = "dtan";
	nbVal = 20000;

	Inc = (2*M_PIl)/nbVal;  /* condering a period of 2 pi rad */

	for (i=0; i<nbVal; i++)
	{
		if ((Inc*i) != (M_PIl/2))
			tabDtan[i] = (Inc * i);
                else
			tabDtan[i] = 0;
	}

	fp = open(F_name,O_RDWR|O_CREAT|O_TRUNC,0777);
        if (!fp)
        {
            	printf("error opening file");
	    	close(fp);
	    	return -1;
        }
        else
        {
		for (i = 0; i<nbVal; i++)
		{
			write(fp,&tabDtan[i],sizeof(double));
		}
		close(fp);
		return 0;
	}
}

int main(int argc, char  *argv[])
{

	if (argc > 1)
	{
		switch ( atoi(argv[1]) )
		{
		case 1:
			if (create_Data_file() == 0)
				printf("Data file created\n");
			else
				printf("problem during tan  data file creation\n");
			break;

		case 2:
			if (create_Result_file() == 0)
				printf("Result file created\n");
			else
				printf("problem during tan result file creation\n");
			break;
		default:
			printf("Bad arglist code for: '%s'\n", argv[0]);
			return -1;
			break;
		}
	}
	else
	{
		if (create_Data_file() != 0)
			printf("problem during tan data file creation\n");
		if (create_Result_file() != 0)
			printf("problem during tan result file creation\n");
	}

  return(0);

}
