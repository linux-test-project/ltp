# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Joerg Vehlow <joerg.vehlow@aox.de>

abs_top_builddir		:= $(abspath $(top_srcdir))

# autotools, *clean, don't require config.mk
ifeq ($(filter autotools %clean,$(MAKECMDGOALS)),)
include $(abs_top_builddir)/include/mk/config.mk
else
-include $(abs_top_builddir)/include/mk/config.mk
endif
