all:
	ln -sf ../testcases/kernel/include/linux_syscall_numbers.h ../../../include/

clean distclean:
	rm -f ../../../include/linux_syscall_numbers.h

install:
	@true

regen:
	./regen.sh

.PHONY: all clean distclean install regen
