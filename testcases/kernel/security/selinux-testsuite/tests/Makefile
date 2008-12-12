REDHAT_RELEASE=$(shell rpm -q redhat-release)

SUBDIRS=domain_trans entrypoint execshare exectrace execute_no_trans fdreceive inherit link mkdir msg open ptrace readlink relabel rename rxdir sem setattr setnice shm sigkill stat sysctl task_create task_setnice task_setscheduler task_getscheduler task_getsid task_getpgid task_setpgid wait file ioctl capable_file capable_net capable_sys dyntrace dyntrans bounds

ifeq (redhat-release-4, $(findstring redhat-release-4, $(REDHAT_RELEASE)))
    SUBDIRS=domain_trans entrypoint execshare exectrace execute_no_trans fdreceive inherit link mkdir msg open ptrace readlink relabel rename rxdir sem setattr setnice shm sigkill stat sysctl task_create task_setnice task_setscheduler task_getscheduler task_getsid task_getpgid task_setpgid wait file ioctl capable_file capable_net capable_sys
endif

ifeq (redhat-release-5, $(findstring redhat-release-5, $(REDHAT_RELEASE)))
    SUBDIRS=domain_trans entrypoint execshare exectrace execute_no_trans fdreceive inherit link mkdir msg open ptrace readlink relabel rename rxdir sem setattr setnice shm sigkill stat sysctl task_create task_setnice task_setscheduler task_getscheduler task_getsid task_getpgid task_setpgid wait file ioctl capable_file capable_net capable_sys dyntrace dyntrans
endif

all: 
	@set -e; for i in $(SUBDIRS); do \
		$(MAKE) -C $$i  all; \
	chcon -R -t test_file_t . ; \
	done

install: all
	mkdir -p ../../../../bin ; 
	@set -e; for i in $(SUBDIRS); do \
		$(MAKE) -C $$i  install; \
	done

clean:
	@set -e; for i in $(SUBDIRS); do \
		$(MAKE) -C $$i  clean; \
	done

