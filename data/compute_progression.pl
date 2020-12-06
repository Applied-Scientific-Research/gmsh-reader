#!/usr/bin/perl
#
# compute the number of nodes and progression for a desired nom sep

my $nomsep = 0.02828;
my $innerdr = 0.005;
my $totalr = 0.5;

my $testexp = 1.047;

my $thick = 0.0;
my $num=0;
my $thisthick = $innerdr;
while ($thick < $totalr) {
  $thick = $thick + $thisthick;
  $thisthick = $thisthick * $testexp;
  $num = $num + 1;
}
$thisthick = $thisthick / $testexp;
print "Exponent ${testexp} results in ${num} cells ${thick} thick and final cell thick ${thisthick}\n";
my $npts = $num + 1;
print "  Gmsh line should read \"Transfinite Curve{x:x} = ${npts} Using Progression ${testexp};\"\n";

