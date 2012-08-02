#include <stdio.h>
#include "config.h"
#include "test.h"

#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif

char *TCID = "dummy";
int TST_TOTAL=1;

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
	tst_brkm(TCONF, NULL, "System doesn't support POSIX capabilities.");
#endif
#else /* capability_h */
	tst_brkm(TCONF, NULL, "System doesn't support sys/capability.h.");
#endif
}
