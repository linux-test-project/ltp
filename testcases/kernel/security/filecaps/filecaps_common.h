#include <limits.h>
#include <stdlib.h>
#include <tso_tmpdir.h>

static char *fifofile;

static const char *get_caps_fifo(void)
{
	if (!fifofile) {
		fifofile = getenv("FIFOFILE");

		if (!fifofile) {
			const char *tmpdir = tst_get_tmpdir_root();

			fifofile = malloc(PATH_MAX);
			snprintf(fifofile, PATH_MAX, "%s/caps_fifo", tmpdir);
		}
	}

	return fifofile;
}
