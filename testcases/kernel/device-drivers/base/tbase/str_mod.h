//str_mod.h
extern int bus_add_device(struct device * dev);
extern int bus_remove_device(struct device *dev);

struct ltpmod_user {

struct device   	*dev;
struct bus_type 	*bus;

};
typedef struct ltpmod_user ltpmod_user_t;


