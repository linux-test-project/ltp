#include <limits.h>
#include <stdlib.h>

static char *fifofile;

static const char *get_caps_fifo(void)
{
	if (!fifofile) {
		fifofile = getenv("FIFOFILE");

		if (!fifofile) {
			const char *tmpdir = getenv("TMPDIR");

			if (!tmpdir)
				tmpdir = "/tmp";
			fifofile = malloc(PATH_MAX);
			snprintf(fifofile, PATH_MAX, "%s/caps_fifo", tmpdir);
		}
	}

	return fifofile;
}
