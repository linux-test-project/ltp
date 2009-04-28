#include <sys/capability.h>

int main()
{
	cap_t cur;
	cur = cap_from_text("all=eip");
	cap_set_proc(cur);
	cap_free(cur);
}
