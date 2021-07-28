# Rules to make sparse tool(s) for inclusion in lib and testcases Makefiles

SPARSE_DIR:= $(abs_top_builddir)/tools/sparse

$(SPARSE_DIR)/sparse-ltp: $(SPARSE_DIR)
	$(MAKE) -C "$^" all

$(SPARSE_DIR): %:
	mkdir -p "$@"
