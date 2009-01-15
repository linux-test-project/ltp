all:
	./unzip_genfile.sh

install:
	ln -f unzip_tests.sh ../../bin/unzip_tests.sh
	ln -f tst_unzip_file.zip ../../bin/tst_unzip_file.zip
	ln -fs $(PWD)/unzip/tst_unzip_file.zip /tmp/tst_unzip_file.zip
	chmod 777 tst_unzip_file.zip

clean:
	rm -f tst_unzip_file.zip
	rm -f /tmp/tst_unzip_file.zip
