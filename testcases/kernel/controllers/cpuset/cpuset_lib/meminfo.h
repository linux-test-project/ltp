#ifndef MEMINFO_H
# define MEMINFO_H

extern int nmems;
extern int mems_nbits;

extern int online_memmask(struct bitmask *memmask);
extern int present_memmask(struct bitmask *memmask);
extern int get_nmems(void);

#endif
