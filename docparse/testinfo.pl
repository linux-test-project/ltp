#!/usr/bin/perl
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

use strict;
use warnings;

use JSON;
use LWP::Simple;
use Cwd qw(abs_path);
use File::Basename qw(dirname);

use constant OUTDIR => dirname(abs_path($0));

sub load_json
{
	my ($fname, $mode) = @_;
	local $/;

	open(my $fh, '<', $fname) or die("Can't open $fname $!");

	return <$fh>;
}

sub log_info
{
	my $msg = shift;
	print STDERR "INFO: $msg\n";
}

sub log_warn
{
	my $msg = shift;
	print STDERR "WARN: $msg\n";
}

sub print_asciidoc_page
{
	my ($fh, $json, $title, $content) = @_;

	print $fh <<EOL;
// -*- mode:doc; -*-
// vim: set syntax=asciidoc:

$title

$content
EOL
}

sub tag_url {
	my ($tag, $value, $scm_url_base) = @_;

    if ($tag eq "CVE") {
        return "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-" . $value;
	}
    if ($tag eq "linux-git") {
        return "https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=" . $value;
	}
    if ($tag eq "fname") {
        return $scm_url_base . $value;
	}
}

sub bold
{
	return "*$_[0]*";
}

sub code
{
	return "+$_[0]+";
}

sub hr
{
	return "\n\n'''\n\n";
}

sub html_a
{
	my ($url, $text) = @_;
	return "$url\[$text\]";
}

sub h1
{
	return "== $_[0]\n";
}

sub h2
{
	return "=== $_[0]\n";
}

sub h3
{
	return "==== $_[0]\n";
}

sub label
{
	return "[[$_[0]]]\n";
}

sub paragraph
{
	return "$_[0]\n\n";
}

sub reference
{
	return "xref:$_[0]\[$_[0]\]" . (defined($_[1]) ? $_[1] : "") . "\n";
}

sub table
{
	return "|===\n";
}

sub table_escape
{
	my $out = $_[0];

	$out =~ s/\|/\\|/g;
	return $out;
}

sub print_defined
{
	my ($key, $val, $val2) = @_;

	if (defined($val)) {
		return paragraph(bold($key) . ": " . $val . (defined($val2) ? " $val2" : ""));
	}
}

sub content_about
{
	my $json = shift;
	my $content;

	$content .= print_defined("URL", $json->{'url'});
	$content .= print_defined("Version", $json->{'version'});
	$content .= print_defined("Default timeout", $json->{'timeout'}, "seconds");

	return $content;
}

sub uniq {
	my %seen;
	grep !$seen{$_}++, @_;
}

sub get_test_names
{
	my @names = @{$_[0]};
	my ($letter, $prev_letter);
	my $content;

	for my $name (sort @names) {
		$letter = substr($name, 0, 1);
		if (defined($prev_letter) && $letter ne $prev_letter) {
			$content .= "\n";
		}

		$content .= reference($name, " ");
		$prev_letter = $letter;
	}
	$content .= "\n";

	return $content;
}

sub get_test_letters
{
	my @names = @{$_[0]};
	my $letter;
	my $prev_letter = "";
	my $content;

	for (@names) {
		$_ = substr($_, 0, 1);
	}
	@names = uniq(@names);

	for my $letter (@names) {
		$content .= reference($letter);
	}
	$content .= "\n";

	return $content;
}

sub tag2title
{
	my $tag = shift;
	return code(".$tag");
}

sub get_filters
{
	my $json = shift;
	my %data;
	while (my ($k, $v) = each %{$json->{'tests'}}) {
		for my $j (keys %{$v}) {

			next if ($j eq 'fname' || $j eq 'doc');

			$data{$j} = () unless (defined($data{$j}));
			push @{$data{$j}}, $k;
		}
	}
	return \%data;
}

# TODO: Handle better .tags (and anything else which contains array)
# e.g. for .tags there could be separate list for CVE and linux-git
# (now it's together in single list).
sub content_filters
{
	my $json = shift;
	my $data = get_filters($json);
	my %h = %$data;
	my $content;

	for my $k (sort keys %$data) {
		my $tag = tag2title($k);
		my ($letter, $prev_letter);
		$content .= h2($tag);
		$content .= paragraph("Tests containing $tag flag.");
		$content .= get_test_names(\@{$h{$k}});
	}

	return $content;
}

sub detect_git
{
	unless (defined $ENV{'LINUX_GIT'} && $ENV{'LINUX_GIT'}) {
		log_warn("kernel git repository not defined. Define it in \$LINUX_GIT");
		return 0;
	}

	unless (-d $ENV{'LINUX_GIT'}) {
		log_warn("\$LINUX_GIT does not exit ('$ENV{'LINUX_GIT'}')");
		return 0;
	}

	my $ret = 0;
	if (system("which git >/dev/null")) {
		log_warn("git not in \$PATH ('$ENV{'PATH'}')");
		return 0;
	}

	chdir($ENV{'LINUX_GIT'});
	if (!system("git log -1 > /dev/null")) {
		log_info("using '$ENV{'LINUX_GIT'}' as kernel git repository");
		$ret = 1;
	} else {
		log_warn("git failed, git not installed or \$LINUX_GIT is not a git repository? ('$ENV{'LINUX_GIT'}')");
	}
	chdir(OUTDIR);

	return $ret;
}

sub content_all_tests
{
	my $json = shift;
	my @names = sort keys %{$json->{'tests'}};
	my $letters = paragraph(get_test_letters(\@names));
	my $has_kernel_git = detect_git();
	my $tmp = undef;
	my $printed = "";
	my $content;

	unless ($has_kernel_git) {
		log_info("Parsing git messages from linux git repository skipped due previous error");
	}

	$content .= paragraph("Total $#names tests.");
	$content .= $letters;
	$content .= get_test_names(\@names);

	for my $name (@names) {
		my $letter = substr($name, 0, 1);

		if ($printed ne $letter) {
			$content .= label($letter);
			$content .= h2($letter);
			$printed = $letter;
		}

		$content .= hr() if (defined($tmp));
		$content .= label($name);
		$content .= h3($name);
		$content .= $letters;

		if (defined($json->{'scm_url_base'}) &&
			defined($json->{'tests'}{$name}{fname})) {
			$content .= paragraph(html_a(tag_url("fname", $json->{'tests'}{$name}{fname},
					$json->{'scm_url_base'}), "source"));
		}

		if (defined $json->{'tests'}{$name}{doc}) {
			for my $doc (@{$json->{'tests'}{$name}{doc}}) {

				# fix formatting for asciidoc [DOCUMENTATION] => *DOCUMENTATION*
				if ($doc =~ s/^\[(.*)\]$/$1/) {
					$doc = paragraph(bold($doc));
				}

				$content .= "$doc\n";
			}
			$content .= "\n";
		}

		if ($json->{'tests'}{$name}{timeout}) {
			if ($json->{'tests'}{$name}{timeout} eq -1) {
				$content .= paragraph("Test timeout is disabled");
			} else {
				$content .= paragraph("Test timeout is $json->{'tests'}{$name}{timeout} seconds");
			}
		} else {
			$content .= paragraph("Test timeout defaults to $json->{'timeout'} seconds");
		}

		my $tmp2 = undef;
		for my $k (sort keys %{$json->{'tests'}{$name}}) {
			my $v = $json->{'tests'}{$name}{$k};
			next if ($k eq "tags" || $k eq "fname" || $k eq "doc");
			if (!defined($tmp2)) {
				$content .= table . "|Key|Value\n\n"
			}

			$content .= "|" . tag2title($k) . "\n|";

			if (ref($v) eq 'ARRAY') {
				# two dimensional array
				if (ref(@$v[0]) eq 'ARRAY') {
					for my $v2 (@$v) {
						$content .= paragraph(table_escape(join(' ', @$v2)));
					}
				} else {
					# one dimensional array
					$content .= table_escape(join(', ', @$v));
				}
			} else {
				# plain content
				$content .= table_escape($v);
			}

			$content .= "\n";

			$tmp2 = 1;
		}
		if (defined($tmp2)) {
			$content .= table . "\n";
		}

		$tmp2 = undef;
		my %commits;

		for my $tag (@{$json->{'tests'}{$name}{tags}}) {
			if (!defined($tmp2)) {
				$content .= table . "|Tags|Info\n"
			}
			my $k = @$tag[0];
			my $v = @$tag[1];
			my $text = $k;

            if ($has_kernel_git && $k eq "linux-git") {
				$text .= "-$v";
				unless (defined($commits{$v})) {
					chdir($ENV{'LINUX_GIT'});
					$commits{$v} = `git log --pretty=format:'%s' -1 $v`;
					chdir(OUTDIR);
				}
				$v = $commits{$v};
			}
			my $a = html_a(tag_url($k, @$tag[1]), $text);
			$content .= "\n|$a\n|$v\n";
			$tmp2 = 1;
		}
		if (defined($tmp2)) {
			$content .= table . "\n";
		}

		$tmp = 1;
	}

	return $content;
}


my $json = decode_json(load_json($ARGV[0]));

my $config = [
    {
		file => "about.txt",
		title => h2("About $json->{'testsuite'}"),
		content => \&content_about,
    },
    {
		file => "filters.txt",
		title => h1("Test filtered by used flags"),
		content => \&content_filters,
    },
    {
		file => "all-tests.txt",
		title => h1("All tests"),
		content => \&content_all_tests,
    },
];

sub print_asciidoc_main
{
	my $config = shift;
	my $file = "metadata.txt";
	my $content;

	open(my $fh, '>', $file) or die("Can't open $file $!");

	$content = <<EOL;
:doctype: inline
:sectanchors:
:toc:

EOL
	for my $c (@{$config}) {
		$content .= "include::$c->{'file'}\[\]\n";
	}
	print_asciidoc_page($fh, $json, h1($json->{'testsuite_short'} . " test catalog"), $content);
}

for my $c (@{$config}) {
	open(my $fh, '>', $c->{'file'}) or die("Can't open $c->{'file'} $!");
	print_asciidoc_page($fh, $json, $c->{'title'}, $c->{'content'}->($json));
}

print_asciidoc_main($config);
