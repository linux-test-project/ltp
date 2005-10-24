SCRIPTS = udp4-multi-diffport01 \
	  udp4-multi-diffport02 udp4-multi-diffport03 \
	  udp4-multi-diffport04 udp4-multi-diffport05 \
	  udp4-multi-diffport06 udp4-multi-diffport07 \
	  udp6-multi-diffport01 \
	  udp6-multi-diffport02 udp6-multi-diffport03 \
	  udp6-multi-diffport04 udp6-multi-diffport05 \
	  udp6-multi-diffport06 udp6-multi-diffport07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
