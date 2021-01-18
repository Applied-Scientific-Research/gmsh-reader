#!/usr/bin/perl
#
# generate a structured mesh of a lid-driven cavity for Omega2D (Hybrid)

my $cavx = 1.0;
my $cavy = 1.0;
my $thick = 0.1;
my $outerdt = 0.02;
my $innerdt = $outerdt * 0.25;
my $dl = $outerdt;
my $geomorder = 1;	# can stay 1 for elements with straight sides

#$innerdr = $ARGV[0];
#$totalr = $ARGV[2];
#$testexp = $ARGV[1];

# how many cells per side?
my $nperside = int(($cavx-2.0*$thick) / $dl + 0.5);
my $ncirc = $nperside;
print "Will use ${nperside} cells along each side, each ${outerdt} thick\n";

# now, more difficult, find out how many cells we need in the radial direction AND the progression

# first, march the exponent up until we get close to the right outer cell size
my $testexp = 1.0;
my ($nrad, $totthick, $lastdr) = &compute_series($innerdt, $testexp, $thick);
while ($lastdr < $outerdt) {
  $testexp = $testexp + 0.01;
  ($nrad, $totthick, $lastdr) = &compute_series($innerdt, $testexp, $thick);
}

# now march backwards until the number of cells jumps by one
$numcells = $nrad;
while ($nrad == $numcells) {
  $testexp = $testexp - 0.001;
  ($nrad, $totthick, $lastdr) = &compute_series($innerdt, $testexp, $thick);
}

# finally, march up again until the number goes back down - now the total thickness is nearly exact
while ($nrad > $numcells) {
  $testexp = $testexp + 0.0001;
  ($nrad, $totthick, $lastdr) = &compute_series($innerdt, $testexp, $thick);
}

my $npts = $nrad + 1;
print "  Gmsh line should read \"Transfinite Curve{x:x} = ${npts} Using Progression ${testexp};\"\n";

# generate the gmsh script file to make this
my $progr = "${npts} Using Progression ${testexp}";
my $templ = "cavity_structured_template.geo";
$command = "sed 's|XSIZ|${cavx}|g; s|YSIZ|${cavy}|g; s|THICK|${thick}|g; s|NCIRC|${ncirc}|g; s|PROGRESSION|${progr}|g; s|ORDER|${geomorder}|g;' < ${templ} > temp.geo";
system $command;

# run that script
$command = "gmsh -2 -format msh4 -o out.msh temp.geo";
system $command;
$command = "gmsh -2 -format su2 -o out.su2 temp.geo";
system $command;

# re-name the mesh file (let the user do that)

exit(0);


sub compute_series {
  my $indr = $_[0];
  my $inexp = $_[1];
  my $intot = $_[2];

  my $thick = 0.0;
  my $num=0;
  my $thisthick = $indr;
  while ($thick < $intot) {
    $thick = $thick + $thisthick;
    $thisthick = $thisthick * $inexp;
    $num = $num + 1;
  }
  $thisthick = $thisthick / $inexp;
  print "  exponent ${inexp} results in ${num} cells ${thick} thick and final cell thick ${thisthick}\n";

  return ($num, $thick, $thisthick)
}

