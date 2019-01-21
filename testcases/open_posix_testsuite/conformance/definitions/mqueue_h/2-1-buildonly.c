/*
  Test for the existence and valid prototype
  of the mq_close function as specified on
  line 9684 of the Base Definitions document
*/

#include <mqueue.h>
#include "posixtest.h"

void test_mq_close_prototype(void)
{
	mqd_t mqdes;
	int err;

	mqdes = 0;

	err = mq_close(mqdes);
	(void)err;
}
