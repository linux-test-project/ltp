SCRIPTS = udp4-multi-diffnic01 \
	  udp4-multi-diffnic02 udp4-multi-diffnic03 \
	  udp4-multi-diffnic04 udp4-multi-diffnic05 \
	  udp4-multi-diffnic06 udp4-multi-diffnic07 \
	  udp6-multi-diffnic01 \
	  udp6-multi-diffnic02 udp6-multi-diffnic03 \
	  udp6-multi-diffnic04 udp6-multi-diffnic05 \
	  udp6-multi-diffnic06 udp6-multi-diffnic07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
