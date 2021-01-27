#!/usr/bin/perl
#
# generate a structured annular mesh for Omega2D (Hybrid)

my $innerrad = 0.5;
my $outerrad = 1.0;

# fix both
my $outerdr = 0.002;
my $innerdr = 0.002;

# fix outer, vary inner
$innerdr = $outerdr * $innerrad / $outerrad;

# fix inner, vary outer
#$outerdr

my $geomorder = 3;	# really, 1 or 2 for this
my $elemorder = 1;	# aim for 1-3 here

#$innerdr = $ARGV[0];
#$totalr = $ARGV[2];
#$testexp = $ARGV[1];

# use the outer dr and elem order to set the number of cells in the circumferential direction
# (so each solution node covers roughly outerdr x outerdr in area)
my $outercelldr = $outerdr * $elemorder;
my $ntheta = 6.28318531 * $outerrad / $outercelldr;
# round up/down to the nearest 4
$ntheta = 4 * int($ntheta*0.25);
#$ntheta = 4 * (int($ntheta*0.25) - 1);
my $outerdx = 6.28318531 * $outerrad / $ntheta;
print "Will use ${ntheta} cells and nodes around the circumference, each ${outerdx} wide\n";

# now, more difficult, find out how many cells we need in the radial direction AND the progression

# first, march the exponent up until we get close to the right outer cell size
my $innercelldr = $innerdr * $elemorder;
my $testexp = 1.0;
my ($nrad, $totthick, $lastdr) = &compute_series($innercelldr, $testexp, $outerrad-$innerrad);
while ($lastdr < $outercelldr) {
  $testexp = $testexp + 0.01;
  ($nrad, $totthick, $lastdr) = &compute_series($innercelldr, $testexp, $outerrad-$innerrad);
}

# now march backwards until the number of cells jumps by one
$numcells = $nrad;
while ($nrad == $numcells) {
  $testexp = $testexp - 0.001;
  ($nrad, $totthick, $lastdr) = &compute_series($innercelldr, $testexp, $outerrad-$innerrad);
}

# finally, march up again until the number goes back down - now the total thickness is nearly exact
while ($nrad > $numcells) {
  $testexp = $testexp + 0.0001;
  ($nrad, $totthick, $lastdr) = &compute_series($innercelldr, $testexp, $outerrad-$innerrad);
}

my $npts = $nrad + 1;
print "  Gmsh line should read \"Transfinite Curve{x:x} = ${npts} Using Progression ${testexp};\"\n";

# generate the gmsh script file to make this
my $progr = "${npts} Using Progression ${testexp}";
my $templ = "annular_structured_template.geo";
my $ncpq = $ntheta / 4;
$command = "sed 's|INNER|${innerrad}|g; s|OUTER|${outerrad}|g; s|NCIRCPERQUAD|${ncpq}|g; s|PROGRESSION|${progr}|g; s|ORDER|${geomorder}|g;' < ${templ} > temp.geo";
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

