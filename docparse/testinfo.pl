#!/usr/bin/perl
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2020-2021 Petr Vorel <pvorel@suse.cz>

use strict;
use warnings;

use JSON qw(decode_json);
use Cwd qw(abs_path);
use File::Basename qw(dirname);

use constant OUTDIR => dirname(abs_path($0));

# tags which expect git tree, also need constant for URL
our @TAGS_GIT = ("linux-git", "linux-stable-git", "glibc-git", "musl-git");

# tags should map these in lib/tst_test.c
use constant LINUX_GIT_URL => "https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=";
use constant LINUX_STABLE_GIT_URL => "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/commit/?id=";
use constant GLIBC_GIT_URL => "https://sourceware.org/git/?p=glibc.git;a=commit;h=";
use constant MUSL_GIT_URL => "https://git.musl-libc.org/cgit/musl/commit/src/linux/clone.c?id=";
use constant CVE_DB_URL => "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-";

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

	if ($tag eq "fname") {
		return $scm_url_base . $value;
	}

	if ($tag eq "CVE") {
		return CVE_DB_URL . $value;
	}

	# *_GIT_URL
	my $key = tag2env($tag) . "_URL";
	if (defined($constant::declared{"main::$key"})) {
		return eval("main::$key") . $value;
	}

	if ('known-fail') {
		return '';
	}

	die("unknown constant '$key' for tag $tag, define it!");
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

	# escape: ] |
	$text =~ s/([]|])/\\$1/g;

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
	my ($link, %args) = @_;

	$args{text} //= $link;
	$args{delimiter} //= "";

	return "xref:$link\[$args{text}\]$args{delimiter}\n";
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

	$content .= print_defined("URL", $json->{'testsuite'}->{'url'});
	$content .= print_defined("Version", $json->{'testsuite'}->{'version'});
	$content .= print_defined("Default timeout", $json->{'defaults'}->{'timeout'}, "seconds");

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

		$content .= reference($name, delimiter => " ");
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

			if ($j eq 'tags') {
				for my $tags (@{$v}{'tags'}) {
					for my $tag (@$tags) {
						my $k2 = $$tag[0];
						my $v2 = $$tag[1];
						$data{$j}{$k2} = () unless (defined($data{$j}{$k2}));
						push @{$data{$j}{$k2}}, $k unless grep{$_ eq $k} @{$data{$j}{$k2}};
					}
				}
			} else {
				push @{$data{$j}}, $k unless grep{$_ eq $k} @{$data{$j}};
			}
		}
	}
	return \%data;
}

sub content_filter
{
	my $k = $_[0];
	my $title = $_[1];
	my $desc = $_[2];
	my $h = $_[3];
	my ($letter, $prev_letter, $content);

	$content = label($k);
	$content .= $title;
	$content .= paragraph("Tests containing $desc flag.");

	$content .= get_test_names(\@{$h});

	return $content;
}

sub content_filters
{
	my $json = shift;
	my $data = get_filters($json);
	my %h = %$data;
	my $content;

	for my $k (sort keys %$data) {
		my $title = tag2title($k);
		if (ref($h{$k}) eq 'HASH') {
			$content .= label($k);
			$content .= h2($title);
			for my $k2 (sort keys %{$h{$k}}) {
				my $title2 = code($k2);
				$content .= content_filter($k2, h3($title2), "$title $title2", $h{$k}{$k2});
			}
		} else {
			$content .= content_filter($k, h2($title), $title, \@{$h{$k}});
		}
	}

	return $content;
}

sub tag2env
{
	my $tag = shift;
	$tag =~ s/-/_/g;
	return uc($tag);
}

sub detect_git
{
	my %data;

	for my $tag (@TAGS_GIT) {
		my $env = tag2env($tag);

		unless (defined $ENV{$env} && $ENV{$env}) {
			log_warn("git repository $tag not defined. Define it in \$$env");
			next;
		}

		unless (-d $ENV{$env}) {
			log_warn("\$$env does not exit ('$ENV{$env}')");
			next;
		}

		if (system("which git >/dev/null")) {
			log_warn("git not in \$PATH ('$ENV{'PATH'}')");
			next;
		}

		chdir($ENV{$env});
		if (!system("git log -1 > /dev/null")) {
			log_info("using '$ENV{$env}' as $env repository");
			$data{$tag} = $ENV{$env};
		} else {
			log_warn("git failed, git not installed or \$$env is not a git repository? ('$ENV{$env}')");
		}
		chdir(OUTDIR);
	}

	return \%data;
}

sub content_all_tests
{
	my $json = shift;
	my @names = sort keys %{$json->{'tests'}};
	my $letters = paragraph(get_test_letters(\@names));
	my $git_url = detect_git();
	my $tmp = undef;
	my $printed = "";
	my $content;

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

		if (defined($json->{'testsuite'}->{'scm_url_base'}) &&
			defined($json->{'tests'}{$name}{fname})) {
			$content .= paragraph(html_a(tag_url("fname", $json->{'tests'}{$name}{fname},
					$json->{'testsuite'}->{'scm_url_base'}), "source"));
		}

		if (defined $json->{'tests'}{$name}{doc}) {
			for my $doc (@{$json->{'tests'}{$name}{doc}}) {

				# fix formatting for asciidoc [DOCUMENTATION] => *Documentation*
				if ($doc =~ s/^\[(.*)\]$/$1/) {
					$doc = paragraph(bold(ucfirst(lc($doc))));
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
			$content .= paragraph("Test timeout defaults to $json->{'defaults'}->{'timeout'} seconds");
		}

		my $tmp2 = undef;
		for my $k (sort keys %{$json->{'tests'}{$name}}) {
			my $v = $json->{'tests'}{$name}{$k};
			next if ($k eq "tags" || $k eq "fname" || $k eq "doc");
			if (!defined($tmp2)) {
				$content .= table . "|Key|Value\n\n"
			}

			$content .= "|" . reference($k, text => tag2title($k)) . "\n|";

			if (ref($v) eq 'ARRAY') {
				# two dimensional array
				if (ref(@$v[0]) eq 'ARRAY') {
					for my $v2 (@$v) {
						# convert NULL to "NULL" string to be printed
						for my $v3 (@$v2) {
							$v3 = "NULL" if (!defined $v3);
						}
						$content .= paragraph(table_escape(join(', ', @$v2)));
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
		my @sorted_tags = sort { $a->[0] cmp $b->[0] } @{$json->{'tests'}{$name}{tags} // []};

		for my $tag (@sorted_tags) {
			if (!defined($tmp2)) {
				$content .= table . "|Tag|Info\n"
			}
			my $k = @$tag[0];
			my $v = @$tag[1];
			my $url;

			if (defined($$git_url{$k})) {
				$commits{$k} = () unless (defined($commits{$k}));
				unless (defined($commits{$k}{$v})) {
					chdir($$git_url{$k});
					$commits{$k}{$v} = `git log --pretty=format:'%s' -1 $v`;
					chdir(OUTDIR);
				}
				$v .= ' ("' . $commits{$k}{$v} . '")';
			}

			$url = tag_url($k, @$tag[1]);
			if ($url) {
				$v = html_a($url, $v);
			}

			# tag value value can be split into more lines if too long
			# i.e. URL in known-fail
			for (@$tag[2 .. $#$tag]) {
				$v .= " $_";
			}

			$content .= "\n|" . reference($k) . "\n|$v\n";
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
		title => h2("About $json->{'testsuite'}->{'name'}"),
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
	print_asciidoc_page($fh, $json, h1($json->{'testsuite'}->{'short_name'} . " test catalog"), $content);
}

for my $c (@{$config}) {
	open(my $fh, '>', $c->{'file'}) or die("Can't open $c->{'file'} $!");
	print_asciidoc_page($fh, $json, $c->{'title'}, $c->{'content'}->($json));
}

print_asciidoc_main($config);
