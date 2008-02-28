#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef PATH_MAX
char fullpath[PATH_MAX];
#else
char fullpath[1024]; /* Guess */
#endif

int FLAG, timer_expired;

int retval;

unsigned int num_line;//??

unsigned int current_shares;

unsigned int total_shares;

unsigned int *shares_pointer;//??

char target[LINE_MAX];

struct dirent 	*dir_pointer;

enum{
	GET_SHARES	=1,
	GET_TASKS
};

inline int error_function(char *msg1, char *msg2);

unsigned int read_shares_file (char *filepath);

int read_file(char *filepath, int action, unsigned int *value);

int scan_shares_files ();

int write_to_file (char * file, const char* mode, unsigned int value);

void signal_handler_alarm (int signal );
