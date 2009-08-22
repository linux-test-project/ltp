
#define __init
#define __user

#define late_initcall(x) typeof(x) x __attribute__((used))
