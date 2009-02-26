all uclinux: ../../../include/linux_syscall_numbers.h

../../../include/linux_syscall_numbers.h: linux_syscall_numbers.h
	ln -sf ../testcases/kernel/include/linux_syscall_numbers.h ../../../include/

clean distclean:
	rm -f linux_syscall_numbers.h
	rm -f ../../../include/linux_syscall_numbers.h

install uclinux_install:
	@true

linux_syscall_numbers.h: *.in regen.sh
	$(MAKE) regen

regen:
	./regen.sh

.PHONY: all clean distclean install regen
