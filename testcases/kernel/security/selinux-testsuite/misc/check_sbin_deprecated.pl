#!/usr/bin/perl

# exit value 0: corecmd_sbin_* is not deprecated
# exit value 2: corecmd_sbin_* is deprecated
# exit value 1: error encountered

my $fnam = "/usr/share/selinux/devel/include/kernel/corecommands.if";

open(FD, "<$fnam") or die "Couldn't open $fnam\n";
my $state=0;

my $ilines = "";

sub check_for_deprecated
{
	my ($ilines) = @_;
	if ($ilines =~ m/deprecated/) {
		exit 2;
	}
	exit 0;
}

while (my $line = <FD>) {
	if ($state == 0) {
		if ($line =~ m/^interface\(`corecmd_sbin_entry_type',`/) {
			$state = 1;
		}
	} elsif ($state == 1) {
		if ($line =~ m/^'\)/) {
			check_for_deprecated($ilines);
		} else {
			$ilines .= $line;
		}
	}
}
print "Error: interface corecmd_sbin_entry_type not found\n";
exit 1;
