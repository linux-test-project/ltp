#include <stdio.h>
#include "config.h"

#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif

int main(void)
{
#if HAVE_SYS_CAPABILITY_H
#ifdef HAVE_LIBCAP
	cap_t cur;
	cur = cap_from_text("all=eip");
	cap_set_proc(cur);
	cap_free(cur);
	return 0;
#else /* libcap */
	printf("System doesn't support POSIX capabilities.\n");
	return 1;
#endif
#else /* capability_h */
	printf("System doesn't support sys/capability.h\n");
	return 1;
#endif
}
