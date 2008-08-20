all: linux_syscall_numbers.h
	ln -sf ../testcases/kernel/include/linux_syscall_numbers.h ../../../include/

clean distclean:
	rm -f linux_syscall_numbers.h
	rm -f ../../../include/linux_syscall_numbers.h

install:
	@true

linux_syscall_numbers.h: *.in
	$(MAKE) regen

regen:
	./regen.sh

.PHONY: all clean distclean install regen
