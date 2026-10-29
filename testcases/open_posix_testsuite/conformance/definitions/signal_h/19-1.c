  /*
     Test the definition of stack_t.
   */

#include <sys/types.h>
#include <signal.h>
#include "posixtest.h"

static stack_t this_type_should_exist, t;
static void *sp;
static size_t size;
static int flags;

int test_main(int argc PTS_ATTRIBUTE_UNUSED, char **argv PTS_ATTRIBUTE_UNUSED)
{
	sp = t.ss_sp;
	size = t.ss_size;
	flags = t.ss_flags;

	return 0;
}
