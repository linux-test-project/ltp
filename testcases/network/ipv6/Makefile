SUBDIRS = `ls */Makefile | sed "s/Makefile//g"`

all:
	@set -e; rm -f ftp6 2>/dev/null;ln -fs ../tcp_cmds/ftp ftp6
	@set -e; rm -f rcp6 2>/dev/null;ln -fs ../tcp_cmds/rcp rcp6
	@set -e; rm -f rlogin6 2>/dev/null;ln -fs ../tcp_cmds/rlogin rlogin6
	@set -e; rm -f rsh6 2> /dev/null;ln -fs ../tcp_cmds/rsh rsh6
	@set -e; rm -f rwho6 2> /dev/null;ln -fs ../tcp_cmds/rwho rwho6
	@set -e; rm -f telnet6 2> /dev/null;ln -fs ../tcp_cmds/telnet telnet6
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
	@rm -f ftp6
	@rm -f rcp6
	@rm -f rlogin6
	@rm -f rsh6
	@rm -f rwho6
	@rm -f telnet6

