# for FC3
include /etc/selinux/config
POLICYSRC = /etc/selinux/$(SELINUXTYPE)/src/policy

# for FC2
#POLICYSRC = /etc/security/selinux/src/policy

load:
	@if [ -d $(POLICYSRC) ]; then \
		install test_*.te $(POLICYSRC)/domains/misc; \
		$(MAKE) -C $(POLICYSRC) clean load; \
	else \
		echo "ERROR: You must have the policy sources installed in $(POLICYSRC)."; \
	fi

cleanup:
	rm -f $(POLICYSRC)/domains/misc/test_*.te
	$(MAKE) -C $(POLICYSRC) clean load
