all:
	$(CC) -o cron_dirs_check cron_dirs_check.c
install:
	ln -f cron01  ../../bin/
	ln -f cron02  ../../bin/
	ln -f cron_illegal_cron_lines ../../bin/
	ln -f cron_pos_tests.sh ../../bin/
	ln -f cron_neg_tests.sh ../../bin/
	ln -f cron_allow01 ../../bin/
	ln -f cron_deny01 ../../bin/
	ln -f cron_dirs_checks01 ../../bin/
	ln -f cron_dirs_check ../../bin/
clean:
	rm -f cron_dirs_check
