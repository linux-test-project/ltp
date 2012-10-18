#define MAXNAMELEN 1024
struct dioattr {
	int d_miniosz, d_maxiosz, d_mem;
};

#define MIN(a,b) ((a)<(b) ? (a):(b))
#define MAX(a,b) ((a)>(b) ? (a):(b))
