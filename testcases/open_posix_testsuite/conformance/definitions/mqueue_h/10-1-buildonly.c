/*
  Test for the existence and valid prototype
  of the mq_timedsend function as specified on
  line 9694 of the Base Definitions document
*/

#include <mqueue.h>
#include <time.h>
#include "posixtest.h"

void test_mq_timedsend_prototype(void)
{
	mqd_t mqdes;
	struct timespec timeout;
	char *msgp;
	int err;
	size_t msg_len;
	unsigned msg_prio;

	mqdes = 0;
	msgp = NULL;
	msg_len = 0;
	msg_prio = 0;

	err = mq_timedsend(mqdes, msgp, msg_len, msg_prio, &timeout);
	(void)err;
}
