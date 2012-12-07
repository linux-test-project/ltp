/*
 *   Copyright (c) International Business Machines  Corp., 2004
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

/*
 * FILE NAME	: dm_test.c
 *
 * PURPOSE	: Define functions and variables common to all DMAPI test cases
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include "dm_test.h"

#define TEST_NAME "dm_test"

int dm_StartingVariation = 0;
int dm_StoppingVariation = INT32_MAX;
int dm_CurrentVariation = 0;
u_int dm_FileLoggingLevel = 0;
u_int dm_TerminalLoggingLevel = 0;
char *dm_LogFileName = "dm_logfile";
u_int dm_PassedVariations = 0;
u_int dm_FailedVariations = 0;
u_int dm_SkippedVariations = 0;
char *dm_TestCaseName = TEST_NAME;
int dm_fdLogFile;
FILE *dm_fpLogFile;
int dm_argc = 0;
char **dm_argv = NULL;
int dm_FileNewlineNeeded;
int dm_TerminalNewlineNeeded;

void dm_ParseCommandLineOptions(int argc, char **argv)
{

	int i;
	char *p;

	if ((p = strrchr(argv[0], '/')) != NULL)
		dm_TestCaseName = ++p;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(&argv[i][1], "runfrom") == 0) {
				if (i < argc - 1 && argv[i + 1][0] != '-')
					dm_StartingVariation = atoi(argv[++i]);
				else
					dm_Error
					    ("invalid/missing runfrom argument\n");
			} else if (strcmp(&argv[i][1], "runto") == 0) {
				if (i < argc - 1 && argv[i + 1][0] != '-')
					dm_StoppingVariation = atoi(argv[++i]);
				else
					dm_Error
					    ("invalid/missing runto argument\n");
			} else if (strcmp(&argv[i][1], "runonly") == 0) {
				if (i < argc - 1 && argv[i + 1][0] != '-') {
					dm_StartingVariation = atoi(argv[++i]);
					dm_StoppingVariation =
					    dm_StartingVariation;
				} else
					dm_Error
					    ("invalid/missing runonly argument\n");
			} else if (strcmp(&argv[i][1], "loglevel") == 0) {
				if (i < argc - 1 && argv[i + 1][0] != '-')
					dm_FileLoggingLevel = atoi(argv[++i]);
				else
					dm_Error
					    ("invalid/missing loglevel argument\n");
			} else if (strcmp(&argv[i][1], "termlevel") == 0) {
				if (i < argc - 1 && argv[i + 1][0] != '-')
					dm_TerminalLoggingLevel =
					    atoi(argv[++i]);
				else
					dm_Error
					    ("invalid/missing termlevel argument\n");
			} else if (strcmp(&argv[i][1], "logname") == 0) {
				if (i < argc - 1 && argv[i + 1][0] != '-')
					dm_LogFileName = argv[++i];
				else
					dm_Error
					    ("invalid/missing filename argument\n");
			} else if (strcmp(&argv[i][1], "?") == 0
				   || strcmp(&argv[i][1], "help") == 0
				   || strcmp(&argv[i][1], "-help") == 0) {
				printf("%s usage:\n", argv[0]);
				printf
				    ("\t-runfrom n: set starting variation to n\n");
				printf
				    ("\t-runto n: set stopping variation to n\n");
				printf("\t-runonly n: run only variation n\n");
				printf
				    ("\t-loglevel n: set file logging level to n\n");
				printf
				    ("\t-termlevel n: set terminal logging level to n\n");
				printf
				    ("\t-logname s: set file log name to s\n");
				exit(0);
			} else if (i < argc - 1 && argv[i + 1][0] != '-')
				i++;
		}
	}

	dm_argc = argc;
	dm_argv = argv;

}

char *dm_GetCommandLineOption(char *option)
{

	int i;

	if (!dm_argc)
		dm_Error
		    ("Cannot get command line option without calling DMOPT_PARSE");

	for (i = 1; i < dm_argc; i++)
		if (dm_argv[i][0] == '-' &&
		    strcmp(&dm_argv[i][1], option) == 0 &&
		    i < dm_argc - 1 && dm_argv[i + 1][0] != '-')
			return dm_argv[i + 1];
	return NULL;

}

void dm_StartLogging(void)
{

	struct utsname buf;
	char version[256];
	struct timeval tv;
	struct tm *pDT = NULL;
	struct tm sDT;
	int i;

	if (dm_fpLogFile)
		dm_Error("Cannot start logging when log file already open");

	dm_fdLogFile =
	    open(dm_LogFileName, O_CREAT | O_APPEND | O_SYNC | O_WRONLY,
		 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if (dm_fdLogFile == -1)
		dm_Error("Unable to open log file %s", dm_LogFileName);

	dm_fpLogFile = fdopen(dm_fdLogFile, "a");

	if (dm_fpLogFile == NULL)
		dm_Error("Unable to fdopen log file %s", dm_LogFileName);

	if (uname(&buf) == -1)
		strcpy(version, "Unknown Linux version");
	else
		sprintf(version, "%s %s", buf.sysname, buf.release);

	if (gettimeofday(&tv, NULL) != -1)
		pDT = (struct tm *)localtime_r(&tv.tv_sec, &sDT);

	if (dm_FileLoggingLevel) {
		fprintf(dm_fpLogFile, "%s running on %s\n", TEST_NAME, version);
		fprintf(dm_fpLogFile, "%s invoked with ", dm_TestCaseName);
		for (i = 1; i < dm_argc; i++)
			fprintf(dm_fpLogFile, "%s ", dm_argv[i]);
		if (pDT)
			fprintf(dm_fpLogFile,
				"\n%s starting at %02u:%02u:%02u on %02u/%02u/%04u\n",
				dm_TestCaseName, pDT->tm_hour, pDT->tm_min,
				pDT->tm_sec, pDT->tm_mon + 1, pDT->tm_mday,
				pDT->tm_year + 1900);
		else
			fprintf(dm_fpLogFile, "\n%s starting\n",
				dm_TestCaseName);

	}

	if (dm_TerminalLoggingLevel) {
		printf("%s running on %s\n", TEST_NAME, version);
		printf("%s invoked with ", dm_TestCaseName);
		for (i = 1; i < dm_argc; i++)
			printf("%s ", dm_argv[i]);
		if (pDT)
			printf
			    ("\n%s starting at %02u:%02u:%02u on %02u/%02u/%04u\n",
			     dm_TestCaseName, pDT->tm_hour, pDT->tm_min,
			     pDT->tm_sec, pDT->tm_mon + 1, pDT->tm_mday,
			     pDT->tm_year + 1900);
		else
			printf("\n%s starting\n", dm_TestCaseName);
	}

}

void dm_StopLogging(void)
{

	struct timeval tv;
	struct tm *pDT = NULL;
	struct tm sDT;
	int ranVariations = 0;
	int percentSuccess = 0;

	if (!dm_fpLogFile)
		dm_Error("Cannot stop logging when log file not already open");

	ranVariations = dm_PassedVariations + dm_FailedVariations;

	if (dm_PassedVariations)
		percentSuccess = (dm_PassedVariations * 100) / ranVariations;

	if (gettimeofday(&tv, NULL) != -1)
		pDT = (struct tm *)localtime_r(&tv.tv_sec, &sDT);

	if (dm_FileLoggingLevel) {
		if (pDT)
			fprintf(dm_fpLogFile,
				"%s stopping at %02u:%02u:%02u on %02u/%02u/%04u\n",
				dm_TestCaseName, pDT->tm_hour, pDT->tm_min,
				pDT->tm_sec, pDT->tm_mon + 1, pDT->tm_mday,
				pDT->tm_year + 1900);
		else
			fprintf(dm_fpLogFile, "%s stopping\n", dm_TestCaseName);

		fprintf(dm_fpLogFile,
			"%s status: %u executed, %u passed, %u failed, %u skipped (%u%%)\n",
			dm_TestCaseName, ranVariations, dm_PassedVariations,
			dm_FailedVariations, dm_SkippedVariations,
			percentSuccess);
	}

	if (dm_TerminalLoggingLevel) {
		if (pDT)
			printf
			    ("%s stopping at %02u:%02u:%02u on %02u/%02u/%04u\n",
			     dm_TestCaseName, pDT->tm_hour, pDT->tm_min,
			     pDT->tm_sec, pDT->tm_mon + 1, pDT->tm_mday,
			     pDT->tm_year + 1900);
		else
			printf("%s stopping\n", dm_TestCaseName);

		printf
		    ("%s status: %u executed, %u passed, %u failed, %u skipped (%u%%)\n",
		     dm_TestCaseName, ranVariations, dm_PassedVariations,
		     dm_FailedVariations, dm_SkippedVariations, percentSuccess);
	}

	fclose(dm_fpLogFile);
	close(dm_fdLogFile);

}

void dm_Error(char *format, ...)
{
	va_list args;
	char fmtmsg[256];

	/*
	 * Format error message including message inserts
	 */
	va_start(args, format);
	vsprintf(fmtmsg, format, args);
	va_end(args);

	/*
	 * Display error message if not detached or Presentation Manager process
	 */
	printf("\n%s fatal error: %s\n", TEST_NAME, fmtmsg);

}

void dm_LogPrintf(u_int level, char *format, ...)
{

	va_list args;

	va_start(args, format);
	if (level <= dm_FileLoggingLevel) {
		fprintf(dm_fpLogFile, "[%s %d %d] ", dm_TestCaseName, getpid(),
			level);
		vfprintf(dm_fpLogFile, format, args);
		dm_FileNewlineNeeded = 1;
	}
	va_end(args);
	va_start(args, format);
	if (level <= dm_TerminalLoggingLevel) {
		printf("[%s %d %d] ", dm_TestCaseName, getpid(), level);
		vprintf(format, args);
		dm_TerminalNewlineNeeded = 1;
	}
	va_end(args);

}

int dm_ExecuteVariation(int var)
{

	if (dm_CurrentVariation)
		dm_Error("Cannot execute variation while variation active\n");
	if (var < dm_StartingVariation || var > dm_StoppingVariation)
		return 0;

	dm_CurrentVariation = var;

	if (dm_FileNewlineNeeded)
		fputc('\n', dm_fpLogFile);
	if (dm_TerminalNewlineNeeded)
		putchar('\n');

	dm_LogPrintf(DMLVL_DEBUG, "Variation %d starting\n", var);

	return 1;

}

void dm_PassVariation(void)
{

	if (!dm_CurrentVariation)
		dm_Error("Cannot pass variation while variation not active\n");

	dm_LogPrintf(DMLVL_DEBUG, "Variation %d passed\n\n",
		     dm_CurrentVariation);
	dm_FileNewlineNeeded = dm_TerminalNewlineNeeded = 0;

	dm_PassedVariations++;

	dm_CurrentVariation = 0;

}

void dm_FailVariation(void)
{

	if (!dm_CurrentVariation)
		dm_Error("Cannot fail variation while variation not active\n");

	dm_LogPrintf(DMLVL_DEBUG, "Variation %d failed\n\n",
		     dm_CurrentVariation);
	dm_FileNewlineNeeded = dm_TerminalNewlineNeeded = 0;

	dm_FailedVariations++;

	dm_CurrentVariation = 0;

}

void dm_SkipVariation(void)
{

	if (!dm_CurrentVariation)
		dm_Error("Cannot skip variation while variation not active\n");

	dm_LogPrintf(DMLVL_DEBUG, "Variation %d skipped\n\n",
		     dm_CurrentVariation);
	dm_FileNewlineNeeded = dm_TerminalNewlineNeeded = 0;

	dm_SkippedVariations++;

	dm_CurrentVariation = 0;

}

void dm_EndVariation_SuccessExpected(char *funcname, int expectedRC,
				     int actualRC)
{

	if (actualRC == expectedRC) {
		DMLOG_PRINT(DMLVL_DEBUG, "%s passed with expected rc = %d\n",
			    funcname, expectedRC);
		DMVAR_PASS();
	} else {
		DMLOG_PRINT(DMLVL_ERR,
			    "%s failed with unexpected rc = %d (errno = %d)\n",
			    funcname, actualRC, errno);
		DMVAR_FAIL();
	}

}

void dm_EndVariation_FailureExpected(char *funcname, int expectedRC,
				     int actualRC, int expectedErrno)
{

	if (actualRC == expectedRC) {
		if (errno == expectedErrno) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s passed with expected rc = %d and expected errno = %d\n",
				    funcname, expectedRC, expectedErrno);
			DMVAR_PASS();
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with expected rc = %d but unexpected errno = %d (expected %d)\n",
				    funcname, expectedRC, errno, expectedErrno);
			DMVAR_FAIL();
		}
	} else {
		DMLOG_PRINT(DMLVL_ERR, "%s failed with unexpected rc = %d\n",
			    funcname, actualRC);
		DMVAR_FAIL();
	}

}

int dm_CheckVariation_SuccessExpected(int expectedRC, int actualRC,
				      dm_eventtype_t expectedEvent,
				      dm_eventtype_t actualEvent)
{

	if (expectedEvent == actualEvent) {
		if (actualRC == expectedRC) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Passed, received expected event %d\n",
				    expectedEvent);
			return DMSTAT_PASS;
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Failed, received expected event %d but unexpected rc = %d (expected %d)\n",
				    expectedEvent, actualRC, expectedRC);
			return DMSTAT_FAIL;
		}
	} else {
		DMLOG_PRINT(DMLVL_ERR,
			    "Failed, received unexpected event %d (expected %d)\n",
			    actualEvent, expectedEvent);
		return DMSTAT_FAIL;
	}

}

int dm_CheckVariation_FailureExpected(int expectedRC, int actualRC,
				      int expectedErrno,
				      dm_eventtype_t expectedEvent,
				      dm_eventtype_t actualEvent)
{

	if (expectedEvent == actualEvent) {
		if (actualRC == expectedRC) {
			if (errno == expectedErrno) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Passed, received expected event %d\n",
					    expectedEvent);
				return DMSTAT_PASS;
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "Failed, received expected event %d but unexpected errno = %d (expected %d)\n",
					    expectedEvent, errno,
					    expectedErrno);
				return DMSTAT_FAIL;
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Failed, received expected event %d but unexpected rc = %d (expected %d)\n",
				    expectedEvent, actualRC, expectedRC);
			return DMSTAT_FAIL;
		}
	} else {
		DMLOG_PRINT(DMLVL_ERR,
			    "Failed, received unexpected event %d (expected %d)\n",
			    actualEvent, expectedEvent);
		return DMSTAT_FAIL;
	}

}

void dm_LogHandle(char *hdl, int len)
{

	int i;
	char outbuf[256], *pch;

	memset(outbuf, 0, sizeof(outbuf));

	for (i = 0, pch = outbuf; i < len; i++, pch += 3)
		sprintf(pch, " %02X", hdl[i]);

	DMLOG_PRINT(DMLVL_DEBUG, "Handle: %s\n", outbuf);

}

/* This static array is for the persistent managed region test */
dm_region_t dm_PMR_regbuf[PMR_NUM_REGIONS] = {
#ifdef MULTIPLE_REGIONS
	{0, 1000, DM_REGION_WRITE}
	,
	{2000, 1000, DM_REGION_TRUNCATE}
	,
	{3005, 995, DM_REGION_READ}
	,
	{5432, 2345, DM_REGION_NOEVENT}
	,
#endif
	{8000, 0, DM_REGION_READ | DM_REGION_WRITE | DM_REGION_TRUNCATE}
};

/* Include implementation-dependent functions and variables */
#include "dm_impl.h"
