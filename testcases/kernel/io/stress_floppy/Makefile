all: 

install: generate
	ln -f stress_floppy ../../../bin
	ln -f *_file ../../../bin
	cp -rf dumpdir ../../../bin


generate:
	./generate.sh

clean:
	rm -rf 10*K_file dumpdir
