
#if defined(__GNUC__) || defined(HAS_LONG_LONG)
typedef long long llse_loff_t;
#else
typedef long      llse_loff_t;
#endif

extern llse_loff_t llse_llseek(unsigned int fd,
                               llse_loff_t offset,
			       unsigned int origin);
