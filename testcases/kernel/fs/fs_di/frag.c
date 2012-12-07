/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2009                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        frag.c                                                        */
/*                                                                            */
/* Description: This piece of code creates two files, and writes 1k data to   */
/*              each file in a loop from datafile. Loop continues till it     */
/*              reaches EOF of data file. In a loop fsync, fclose is called,  */
/*              to create fragmented files.                                   */
/*                                                                            */
/* Author:      Jyoti Vantagodi jyotiv@linux.vnet.ibm.com                     */
/*                                                                            */
/* History:     Created-Jul 22 2009-Jyoti Vantagodi jyotiv@linux.vnet.ibm.com */
/*                                                                            */
/******************************************************************************/

#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>

FILE *fp_data;			/* File pointer for data file */
FILE *fp_frag1;			/* File pointer for fragmented file 1 */
FILE *fp_frag2;			/* File pointer for fragmented file 2 */

int main(int argc, char *argv[])
{
	int bytes_read = 0, bytes_written = 0, fd1 = -1, fd2 = -1;
	char buff[1024], frag_file1[100], frag_file2[100];

	if (argc != 3) {
		printf("Needs to pass two arguments..\n");
		return -1;
	}
	fp_data = fopen(argv[1], "r");
	if (!fp_data) {
		perror("fopen");
		printf("Error opening datafile \n");
		return 1;
	}
	strcpy(frag_file1, argv[2]);
	strcat(frag_file1, "/frag1");

	strcpy(frag_file2, argv[2]);
	strcat(frag_file2, "/frag2");
	do {
		fp_frag1 = fopen(frag_file1, "a+");
		if (!fp_frag1) {
			printf("Error opening fragfile \n");
			return -1;
		}
		fp_frag2 = fopen(frag_file2, "a+");
		if (!fp_frag2) {
			perror("fwrite");
			printf("Error opening fragfile \n");
			return -1;
		}
		bytes_read = fread(buff, 1, 1024, fp_data);
		if (bytes_read < 0) {
			perror("fread");
			printf("Error reading data file\n");
			return -1;
		}
		bytes_written = fwrite(buff, 1, bytes_read, fp_frag1);
		if (bytes_read != bytes_written) {
			perror("fwrite");
			printf("Error in writing data\n");
			return -1;
		}
		bytes_written = fwrite(buff, 1, bytes_read, fp_frag2);
		if (bytes_read != bytes_written) {
			perror("fwrite");
			printf("Error in writing data\n");
			return -1;
		}
		fd1 = fileno(fp_frag1);
		fd2 = fileno(fp_frag2);

		fsync(fd1);
		fsync(fd2);
		fclose(fp_frag1);
		fclose(fp_frag2);

		if (bytes_read < 1024)
			break;
	} while (1);
	fclose(fp_data);
	return 0;
}
