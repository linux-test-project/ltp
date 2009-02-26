# Only build and install if we have the zip and unzip commands
HAVE_ZIPTOOLS=$(shell which zip && which unzip && echo t)
ifeq ($(HAVE_ZIPTOOLS),t)
BINS := unzip_tests.sh
DEPS := tst_unzip_file.zip $(BINS)
else
BINS :=
DEPS :=
endif

all: $(DEPS)

tst_unzip_file.zip: tst_unzip.dir
	zip -r tst_unzip_file.zip tst_unzip.dir && chmod 777 tst_unzip_file.zip

# This target should be auto-removed by Make
.INTERMEDIATE: tst_unzip.dir
tst_unzip.dir: unzip_genfile.sh
	./unzip_genfile.sh tst_unzip.dir

install: all
	for i in $(BINS) ; do \
		ln -f $$i ../../bin/$$i ; \
	done

clean:
	rm -rf tst_unzip.dir tst_unzip_file.zip
