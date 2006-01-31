all: cron_dirs_check

cron_dirs_check:
	$(CC) $(CROSS_CFLAGS) $(LDFLAGS) -o cron_dirs_check cron_dirs_check.c

install:
	chmod 755 cron_tests.sh
	@ln -f cron_tests.sh ../../bin/cron_tests.sh
	@ln -f cron02  ../../bin/cron02
	@ln -f cron03  ../../bin/cron03
	@ln -f cron_illegal_cron_lines ../../bin/cron_illegal_cron_lines
	@ln -f cron_pos_tests.sh ../../bin/cron_pos_tests.sh
	@ln -f cron_neg_tests.sh ../../bin/cron_neg_tests.sh
	@ln -f cron_allow01 ../../bin/cron_allow01
	@ln -f cron_deny01 ../../bin/cron_deny01
	@ln -f cron_dirs_checks01 ../../bin/cron_dirs_checks01
	@ln -f cron_dirs_check ../../bin/cron_dirs_check
clean:
	rm -f cron_dirs_check

