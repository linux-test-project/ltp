#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define LTP_GET_TMP_FILENAME(target, prefix) \
    snprintf(target, sizeof(target), \
    "%s/" prefix "_pid-%d", ltp_get_tmpdir(), getpid());

static inline const char *ltp_get_tmpdir(void)
{
    const char *tmpdir_env;
    tmpdir_env = getenv("TMPDIR");
    return tmpdir_env ? tmpdir_env : "/tmp";
}
