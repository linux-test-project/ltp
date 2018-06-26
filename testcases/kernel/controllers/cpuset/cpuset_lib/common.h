#ifndef COMMON_H
#define COMMON_H

#define UNUSED __attribute__ ((unused))

#define OPT_MISSING(prog, opt) do {				\
	fprintf(stderr, "%s: option -%c ", prog, opt);		\
	fprintf(stderr, "requires an argument\n");		\
	usage(prog, 1);						\
} while (0)

#define OPT_COLLIDING(prog, opt1, opt2) do {			\
	fprintf(stderr,						\
		"%s: option -%c collides with option -%c\n",	\
		prog, opt1, opt2);				\
	usage(prog, 1);						\
} while (0)

#define ARG_WRONG(prog, opt, arg) do {				\
	fprintf(stderr, "%s: Wrong argument -%c %s\n",		\
		prog, opt, arg);				\
	usage(prog, 1);						\
} while (0)

#define USECS_PER_SEC	1000000
#define USECS_PER_MSEC	1000
#define NSECS_PER_MSEC	1000000
#define MSECS_PER_SEC	1000
#define NSECS_PER_USEC	1000
#define NSECS_PER_SEC	(USECS_PER_SEC * NSECS_PER_USEC)

#define DECIMAL		10
#define BUFFSIZE	512
#define DEFBITMASKSIZE	64

#ifndef PATH_MAX
# define PATH_MAX	1024
#endif

#define while_each_childdir(basepath, p_relpath, c_relpath, c_pathlen)	\
{									\
	struct dirent *direntp;						\
	DIR *dp;							\
	struct stat st;							\
	char fullpath[PATH_MAX];					\
	int pathlen;							\
	int start = 0;							\
									\
	if (basepath[strlen(basepath) - 1] == '/'			\
		&& p_relpath[0] == '/')					\
		start = 1;						\
									\
	snprintf(fullpath, sizeof(fullpath), "%s%s", basepath,		\
		 &p_relpath[start]);					\
	pathlen = strlen(fullpath);					\
									\
	if ((dp = opendir(fullpath)) == NULL)				\
		return -1;						\
									\
	while ((direntp = readdir(dp)) != NULL) {			\
		if (!strcmp(direntp->d_name, ".")			\
			|| !strcmp(direntp->d_name, ".."))		\
			continue;					\
		if (fullpath[pathlen - 1] == '/') {			\
			fullpath[pathlen - 1] = '\0';			\
			pathlen--;					\
		}							\
		sprintf(fullpath + pathlen, "/%s", direntp->d_name);	\
		stat(fullpath, &st);					\
		if (S_ISDIR(st.st_mode)) {				\
			start = strlen(basepath);			\
			if (basepath[start - 1] == '/')			\
				start--;				\
			snprintf(c_relpath, c_pathlen, "%s",		\
					fullpath + start);

#define	end_while_each_childdir						\
		}							\
	}								\
									\
	closedir(dp);							\
}

#endif
