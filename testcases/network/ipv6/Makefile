SUBDIRS =  echo6 finger6 perf_lan6 ping6 sendfile6 tcpdump6

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
	ln -s ../tcp_cmds/ftp ftp6
	ln -s ../tcp_cmds/rcp rcp6
	ln -s ../tcp_cmds/rlogin rlogin6
	ln -s ../tcp_cmds/rsh rsh6
	ln -s ../tcp_cmds/rwho rwho6
	ln -s ../tcp_cmds/telnet telnet6

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
	rm -f ftp6
	rm -f rcp6
	rm -f rlogin6
	rm -f rsh6
	rm -f rwho6
	rm -f telnet6

