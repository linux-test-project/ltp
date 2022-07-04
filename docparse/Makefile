# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/functions.mk

ifeq ($(METADATA_GENERATOR),asciidoctor)
METADATA_GENERATOR_CMD := asciidoctor
METADATA_GENERATOR_PARAMS := -d book metadata.txt
METADATA_GENERATOR_PARAMS_HTML := -b xhtml
METADATA_GENERATOR_PARAMS_PDF := -b pdf -r asciidoctor-pdf
else ifeq ($(METADATA_GENERATOR),asciidoc)
METADATA_GENERATOR_CMD := a2x
METADATA_GENERATOR_PARAMS := --xsltproc-opts "--stringparam toc.section.depth 1" -d book -L  --resource="$(PWD)" metadata.txt
METADATA_GENERATOR_PARAMS_HTML := -f xhtml
METADATA_GENERATOR_PARAMS_PDF := -f pdf
METADATA_GENERATOR_PARAMS_HTML_CHUNKED := -f chunked
else ifeq ($(METADATA_GENERATOR),)
$(error 'METADATA_GENERATOR' not configured, run ./configure in the root directory)
else
$(error '$(METADATA_GENERATOR)' not supported, only asciidoctor and asciidoc are supported)
endif

ifdef VERBOSE
METADATA_GENERATOR_PARAMS += -v
endif

CLEAN_TARGETS		:= *.css *.js *.txt

ifeq ($(WITH_METADATA_HTML),yes)
MAKE_TARGETS		+= metadata.html
ifneq ($(METADATA_GENERATOR_PARAMS_HTML_CHUNKED),)
MAKE_TARGETS		+= metadata.chunked
endif
endif

ifeq ($(WITH_METADATA_PDF),yes)
MAKE_TARGETS		+= metadata.pdf
endif

INSTALL_DIR = metadata
INSTALL_TARGETS = *.css *.js

ifndef METADATA_GENERATOR
METADATA_GENERATOR := asciidoctor
endif

txt: ${abs_top_builddir}/metadata/ltp.json
	$(abs_srcdir)/testinfo.pl $<

ifeq ($(WITH_METADATA_HTML),yes)
metadata.html: txt
	$(METADATA_GENERATOR_CMD) $(METADATA_GENERATOR_PARAMS) $(METADATA_GENERATOR_PARAMS_HTML)

ifneq ($(METADATA_GENERATOR_PARAMS_HTML_CHUNKED),)
metadata.chunked: txt
	$(METADATA_GENERATOR_CMD) $(METADATA_GENERATOR_PARAMS) $(METADATA_GENERATOR_PARAMS_HTML_CHUNKED)
endif
endif

ifeq ($(WITH_METADATA_PDF),yes)
metadata.pdf: txt
	$(METADATA_GENERATOR_CMD) $(METADATA_GENERATOR_PARAMS) $(METADATA_GENERATOR_PARAMS_PDF)
endif

include $(top_srcdir)/include/mk/generic_leaf_target.mk
