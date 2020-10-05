#!/usr/bin/perl
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>

use strict;
use warnings;

use JSON;
use Data::Dumper;

sub load_json
{
	my ($fname) = @_;
	local $/;

	open(my $fh, '<', $fname) or die("Can't open $fname $!");

	return <$fh>;
}

sub query_flag
{
	my ($json, $flag) = @_;

	my $tests = $json->{'tests'};

	foreach my $key (sort(keys %$tests)) {
		if ($tests->{$key}->{$flag}) {
			if ($tests->{$key}->{$flag} eq "1") {
				print("$key\n");
			} else {
				print("$key:\n" . Dumper($tests->{$key}->{$flag}) . "\n");
			}
		}
	}
}

my $json = decode_json(load_json($ARGV[0]));

query_flag($json, $ARGV[1]);
