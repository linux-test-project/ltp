# Copyright (c) Linux Test Project, 2024-2025
# SPDX-License-Identifier: GPL-2.0-or-later

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

PYTHON := python3
VENV_DIR := .venv

# only fish and bash/zsh supported
VENV_CMD := if [ "x${FISH_VERSION}" != "x" ]; then . $(VENV_DIR)/bin/activate.fish; else . $(VENV_DIR)/bin/activate; fi

RUN_VENV := if [ -d $(VENV_DIR) ]; then $(VENV_CMD); fi

$(VENV_DIR):
	$(PYTHON) -m virtualenv $(VENV_DIR)
	$(VENV_CMD) && pip install -r requirements.txt

.PHONY: setup
setup: $(VENV_DIR)

${abs_top_builddir}/metadata/ltp.json:
	$(MAKE) -C ${abs_top_builddir}/metadata

all: ${abs_top_builddir}/metadata/ltp.json
	$(RUN_VENV); sphinx-build -b html . html

spelling:
	$(RUN_VENV); sphinx-build -b spelling -d build/doctree . build/spelling

clean:
	rm -rf html/ build/ _static/syscalls.rst _static/tests.rst syscalls.tbl \
		${abs_top_builddir}/metadata/ltp.json

distclean: clean
	rm -rf $(VENV_DIR)
