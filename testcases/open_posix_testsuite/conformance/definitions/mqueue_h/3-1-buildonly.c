/*
   Test for the existence and valid prototype
   of the mq_getattr function as specified on
   line 9685 of the Base Definitions document
*/

#include <mqueue.h>
#include "posixtest.h"

void test_mq_getattr_prototype(void)
{
	mqd_t mqdes;
	struct mq_attr mqs;
	int err;

	mqdes = 0;

	err = mq_getattr(mqdes, &mqs);
	(void)err;
}
