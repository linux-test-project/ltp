typedef int (*cmp_t)(void*,void*);       /* for function pointer casts */

extern void register_sort_function (int dir, cmp_t func);
extern void reset_sort_options(void);
extern int mult_lvl_cmp(void* a, void* b);
extern const char *parse_sort_opt(const char* opt);
extern const char *parse_long_sort(const char* opt);

