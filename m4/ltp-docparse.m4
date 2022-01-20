dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

AC_DEFUN([LTP_CHECK_METADATA_GENERATOR_ASCIIDOCTOR], [
	AC_MSG_NOTICE(checking asciidoctor as metadata generator)
	AC_PATH_TOOL(asciidoctor, "asciidoctor")
	metadata_generator_html=$asciidoctor
	# pdf requires both asciidoctor and asciidoctor-pdf
	if test "x$metadata_generator_html" != x; then
		AC_PATH_TOOL(asciidoctor_pdf, "asciidoctor-pdf")
		metadata_generator_pdf=$asciidoctor_pdf
	fi
])

AC_DEFUN([LTP_CHECK_METADATA_GENERATOR_ASCIIDOC], [
	AC_MSG_NOTICE(checking asciidoc as metadata generator)
	AC_PATH_TOOL(a2x, "a2x")
	if test "x$a2x" != x; then
		version="`$a2x --version | cut -d ' ' -f2 `"
		AX_COMPARE_VERSION([$version], lt, 9, [
		AC_MSG_WARN([a2x unsupported version: $version. Use a2x >= 9])
		a2x=
		])
	fi
	metadata_generator_html=$a2x
	# pdf requires both asciidoc and dblatex
	if test "x$metadata_generator_html" != x; then
		AC_PATH_TOOL(dblatex, "dblatex")
		metadata_generator_pdf=$dblatex
	fi
])

AC_DEFUN([LTP_DOCPARSE], [
with_metadata=no
with_metadata_html=no
with_metadata_pdf=no

if test "x$enable_metadata" != xyes; then
	enable_metadata_html=no
	enable_metadata_pdf=no
	with_metadata_generator=none
fi

if test "x$enable_metadata_html" = xyes -o "x$enable_metadata_pdf" = xyes; then
	AX_PROG_PERL_MODULES(Cwd File::Basename JSON LWP::Simple)
fi

if test "x$ax_perl_modules_failed" = x0; then
	if test "x$with_metadata_generator" = xasciidoctor -o "x$with_metadata_generator" = xdetect; then
		LTP_CHECK_METADATA_GENERATOR_ASCIIDOCTOR
	elif test "x$with_metadata_generator" = xasciidoc; then
		LTP_CHECK_METADATA_GENERATOR_ASCIIDOC
	else
		AC_MSG_ERROR([invalid metadata generator '$with_metadata_generator', use --with-metadata-generator=asciidoc|asciidoctor])
	fi

	# autodetection: check also Asciidoc
	if test "x$with_metadata_generator" = xdetect; then
		with_metadata_generator='asciidoctor'
		# problems with Asciidoctor: (html enabled && not found) || (pdf enabled && not found) => try Asciidoc
		if test "x$enable_metadata_html" = xyes -a "x$metadata_generator_html" = x ||
			test "x$enable_metadata_pdf" = xyes -a "x$metadata_generator_pdf" = x; then
			backup_html="$metadata_generator_html"
			backup_pdf="$metadata_generator_pdf"
			AC_MSG_NOTICE(missing some dependencies for Asciidoctor => trying Asciidoc)
			with_metadata_generator='asciidoc'
			LTP_CHECK_METADATA_GENERATOR_ASCIIDOC
			# prefer Asciidoctor if it's not worse than Asciidoc
			# (html not enabled || asciidoctor html found || asciidoc html not found) && (pdf ...)
			if test "x$enable_metadata_html" != xyes -o "x$backup_html" != x -o "x$metadata_generator_html" = x &&
				test "x$enable_metadata_pdf" != xyes -o "x$backup_pdf" != x -o "x$metadata_generator_pdf" = x; then
				with_metadata_generator='asciidoctor'
				metadata_generator_html="$backup_html"
				metadata_generator_pdf="$backup_pdf"
			fi
		fi
		if test "x$metadata_generator_html" != x -o "x$metadata_generator_pdf" != x; then
			AC_MSG_NOTICE(choosing $with_metadata_generator for metadata generation)
		fi
	fi

	if test "x$enable_metadata_html" = xno; then
		AC_MSG_NOTICE(HTML metadata generation disabled)
	elif test "x$metadata_generator_html" != x; then
		with_metadata_html=yes
	fi

	if test "x$enable_metadata_pdf" = xno; then
		AC_MSG_NOTICE(PDF metadata generation disabled)
	elif test "x$metadata_generator_pdf" != x; then
		with_metadata_pdf=yes
	fi
fi

reason="metadata generation skipped due missing suitable generator"
hint="specify correct generator with --with-metadata-generator=asciidoc|asciidoctor or use --disable-metadata|--disable-metadata-html|--disable-metadata-pdf"

if test -z "$ax_perl_modules_failed"; then
	AC_MSG_NOTICE(metadata generation disabled)
elif test "x$ax_perl_modules_failed" = x1; then
	AC_MSG_WARN(metadata generation skipped due missing required Perl modules)
elif test "x$with_metadata_html" = xno -a "x$with_metadata_pdf" = xno; then
	AC_MSG_WARN([$reason, $hint])
else
	with_metadata=yes
	AC_SUBST(METADATA_GENERATOR, $with_metadata_generator)
	if test "x$with_metadata_html" = xno -a "x$enable_metadata_html" = xyes; then
		AC_MSG_WARN([HTML $reason, $hint])
	fi
	if test "x$with_metadata_pdf" = xno -a "x$enable_metadata_pdf" = xyes; then
		AC_MSG_WARN([PDF $reason, $hint])
	fi
fi

AC_SUBST(WITH_METADATA, $with_metadata)
AC_SUBST(WITH_METADATA_HTML, $with_metadata_html)
AC_SUBST(WITH_METADATA_PDF, $with_metadata_pdf)
])
