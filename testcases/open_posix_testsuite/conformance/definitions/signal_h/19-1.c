  /*
     Test the definition of stack_t.
   */

#include <sys/types.h>
#include <signal.h>

stack_t this_type_should_exist, t;
void *sp;
size_t size;
int flags;

int main(void)
{
	sp = t.ss_sp;
	size = t.ss_size;
	flags = t.ss_flags;

	return 0;
}
