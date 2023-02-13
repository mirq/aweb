#!/usr/bin/perl -w

# replace all instances of string $a with $b

# args infile outfile a b

use strict;

use vars qw($infile $outfile $a $b);

if ($#ARGV < 3) { die "called bad";}

$infile = $ARGV[0];
$outfile = $ARGV[1];
$a = $ARGV[2];
$b = $ARGV[3];

open IN, "<$infile" or die "couldn't open $infile";
open OUT, ">$outfile" or die "couldn't open $outfile";

while ( my $line = <IN>)
{
    $line =~ s/$a/$b/g;
    print OUT $line;
}

close IN;
close OUT;




