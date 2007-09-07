CFLAGS = -Wall

all:
	chmod 755 crash_cmds distro master setup summary sysinfo test tests verify
	./distro
	make -C tools all
	./setup

clean:
	make -C tools clean
	rm -f config control crontab testlists/lkdtm testfile
	mv -f /etc/sysconfig/kdump.orig /etc/sysconfig/kdump 2> /dev/null
	crontab -r
