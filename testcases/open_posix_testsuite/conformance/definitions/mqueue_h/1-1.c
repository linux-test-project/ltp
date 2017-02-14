/*
 * Ngie Cooper, August 2010
 *
 * Verify that mq_flags, mq_maxmsg, mq_msgsize, and mq_curmsgs exist in the
 * struct mq_attr structure and are long integers as per the
 * IEEE Std 1003.1-2008 spec.
 */

#include <limits.h>
#include <mqueue.h>
#include <stdio.h>
#include "posixtest.h"

#define TEST_LIMITS(structure_and_field) do {			\
		printf("%s..\n", # structure_and_field);	\
		printf("\t.. -LONG_MIN\n");			\
		structure_and_field = -LONG_MIN;		\
		if (structure_and_field != -LONG_MIN) {		\
			return (PTS_FAIL);			\
		}						\
		printf("\t.. LONG_MAX\n");			\
		structure_and_field = LONG_MAX;			\
		if (structure_and_field != LONG_MAX) {		\
			return (PTS_FAIL);			\
		}						\
		printf("\t.. -(LONG_MIN+1)\n");			\
		structure_and_field = -(LONG_MIN+1);		\
		if (structure_and_field != LONG_MAX) {		\
			return (PTS_FAIL);			\
		}						\
		structure_and_field = LONG_MAX+1;		\
		printf("\t.. (LONG_MAX+1)\n");			\
		if (structure_and_field != LONG_MIN) {		\
			return (PTS_FAIL);			\
		}						\
	} while (0)

int main()
{
	struct mq_attr mqs;

	TEST_LIMITS(mqs.mq_flags);
	TEST_LIMITS(mqs.mq_maxmsg);
	TEST_LIMITS(mqs.mq_msgsize);
	TEST_LIMITS(mqs.mq_curmsgs);

	printf("PASSED!\n");

	return (PTS_PASS);

}
