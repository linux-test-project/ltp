all: page-poisoning

page-poisoning: page-poisoning.o

../../bin/page-poisoning: page-poisoning
	cp page-poisoning ../../bin

install: ../../bin/page-poisoning

clean:
	rm -f page-poisoning *.o
