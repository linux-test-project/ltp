include /etc/selinux/config
SELINUX_SRC=/etc/selinux/$(SELINUXTYPE)/src/policy

# for Fedora Core 2
# SELINUX_SRC=/etc/security/selinux/src/policy

load:
	@if [ -d $(SELINUX_SRC) ]; then \
		cp test_* $(SELINUX_SRC)/domains/misc/; \
		$(MAKE) -C $(SELINUX_SRC) -W users load; \
	else \
		echo "ERROR: You must have selinux-policy-targeted-sources installed."; \
	fi

cleanup:
	rm -f $(SELINUX_SRC)/domains/misc/test_*
	$(MAKE) -C $(SELINUX_SRC) -W users load

