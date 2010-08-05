/*
  Test for the existence and valid prototype
  of the mq_receive function as specified on
  line 9688 of the Base Definitions document
*/

#include <mqueue.h>
#include "posixtest.h"

void test_mq_receive_prototype(void)
{
	mqd_t mqdes;
	ssize_t msg_size;
	size_t msg_len;
	char *msgp;
	unsigned msg_prio;

	msg_size = mq_receive(mqdes, msgp, msg_len, &msg_prio);

}
