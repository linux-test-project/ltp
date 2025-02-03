# Copyright (c) Linux Test Project, 2024-2025
# SPDX-License-Identifier: GPL-2.0-or-later

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

${abs_top_builddir}/metadata/ltp.json:
	$(MAKE) -C ${abs_top_builddir}/metadata

all: ${abs_top_builddir}/metadata/ltp.json
	sphinx-build -b html . html

spelling:
	sphinx-build -b spelling -d build/doctree . build/spelling

clean:
	rm -rf html/ build/ _static/syscalls.rst _static/tests.rst syscalls.tbl
