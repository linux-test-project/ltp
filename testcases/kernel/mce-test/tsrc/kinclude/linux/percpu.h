
#define DECLARE_PER_CPU(x,y)
#define BITS_PER_LONG (sizeof(long)*8)
#define DECLARE_BITMAP(x,y) unsigned long x[((y) + BITS_PER_LONG - 1) / BITS_PER_LONG];
#define MAX_NR_BANKS 32
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

struct cpuinfo_x86;

