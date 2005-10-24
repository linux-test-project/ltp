SCRIPTS = udp4-multi-diffip01 \
	  udp4-multi-diffip02 udp4-multi-diffip03 \
	  udp4-multi-diffip04 udp4-multi-diffip05 \
	  udp4-multi-diffip06 udp4-multi-diffip07 \
	  udp6-multi-diffip01 \
	  udp6-multi-diffip02 udp6-multi-diffip03 \
	  udp6-multi-diffip04 udp6-multi-diffip05 \
	  udp6-multi-diffip06 udp6-multi-diffip07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
