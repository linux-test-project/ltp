#include <stdio.h>
#include "config.h"

#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif

#define END \
	printf("System doesn't support POSIX capabilities.\n"); \
	return 1

int main()
{
#if HAVE_SYS_CAPABILITY_H
	cap_t cur;
#if HAVE_DECL_CAP_SET_PROC
#if HAVE_DECL_CAP_FROM_TEXT
	cur = cap_from_text("all=eip");
	cap_set_proc(cur);
#else
	END;
#endif
#else
	END;
#endif
#if HAVE_DECL_CAP_FREE
	cap_free(cur);
#else
	END;
#endif
#else
	END;
#endif
	return 0;
}
