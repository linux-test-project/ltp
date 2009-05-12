POLICYDEVEL = /usr/share/selinux/devel
SEMODULE = /usr/sbin/semodule

TARGETS=test_global.te test_capable_file.te test_capable_net.te \
test_capable_sys.te test_dyntrace.te test_dyntrans.te test_entrypoint.te \
test_execshare.te test_exectrace.te test_execute_no_trans.te \
test_fdreceive.te test_file.te test_inherit.te test_ioctl.te test_ipc.te \
test_link.te test_mkdir.te test_open.te test_ptrace.te test_readlink.te \
test_relabel.te test_rename.te test_rxdir.te test_setattr.te test_setnice.te \
test_sigkill.te test_stat.te test_sysctl.te test_task_create.te \
test_task_getpgid.te test_task_getsched.te test_task_getsid.te \
test_task_setpgid.te test_task_setsched.te test_transition.te test_wait.te 

load:
	@if [ -d $(POLICYDEVEL) ]; then \
		set -e; rm -f test_policy.te; for i in $(TARGETS); do cat $$i >> test_policy.te; done; \
		cp test_policy.* $(POLICYDEVEL); \
		$(MAKE) -C $(POLICYDEVEL) clean test_policy.pp; \
		$(SEMODULE) -i $(POLICYDEVEL)/test_policy.pp; \
	else \
		echo "ERROR: You must have selinux-policy-devel installed."; \
	fi
	
cleanup:
	$(SEMODULE) -r test_policy
	rm -f $(POLICYDEVEL)/test_policy.* 
	rm -f test_policy.te 

