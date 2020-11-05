// -----------------------------------------------------------------------------
//
//  From Gmsh GEO tutorial 4
//
//  Structured annular mesh
//
// -----------------------------------------------------------------------------

// As usual, we start by defining some variables:

xc = 0.0;
yc = 0.0;
r0 = 0.5;
r1 = 1.0;
ncirc = 1.65*r0/0.05;
nrad = (r1-r0)/0.05;

// We can use all the usual mathematical functions (note the capitalized first
// letters), plus some useful functions like Hypot(a, b) := Sqrt(a^2 + b^2):

// Then we define some points and some lines using these variables:

Point(1) = {xc,    yc,    0};
Point(2) = {xc+r0, yc,    0};
Point(3) = {xc,    yc+r0, 0};
Point(4) = {xc-r0, yc,    0};
Point(5) = {xc,    yc-r0, 0};

Point(6) = {xc+r1, yc,    0};
Point(7) = {xc,    yc+r1, 0};
Point(8) = {xc-r1, yc,    0};
Point(9) = {xc,    yc-r1, 0};
Point(10)= {xc,    yc,    0};

Circle(1) = {2,1,3};
Circle(2) = {3,1,4};
Circle(3) = {4,1,5};
Circle(4) = {5,1,2};

Circle(5) = {6,10,7};
Circle(6) = {7,10,8};
Circle(7) = {8,10,9};
Circle(8) = {9,10,6};
Transfinite Curve{1:8} = ncirc;

Line(11) = {2, 6};
Line(12) = {3, 7};
Line(13) = {4, 8};
Line(14) = {5, 9};
Transfinite Curve{11:14} = nrad Using Progression 1.1;

Curve Loop(11) = {11, 5, -12, -1};
Plane Surface(12) = {11};
Transfinite Surface{12};
Recombine Surface{12};

Curve Loop(13) = {12, 6, -13, -2};
Plane Surface(14) = {13};
Transfinite Surface{14};
Recombine Surface{14};

Curve Loop(15) = {13, 7, -14, -3};
Plane Surface(16) = {15};
Transfinite Surface{16};
Recombine Surface{16};

Curve Loop(17) = {14, 8, -11, -4};
Plane Surface(18) = {17};
Transfinite Surface{18};
Recombine Surface{18};

Mesh.ElementOrder = 2;
Mesh.SecondOrderLinear = 0;


// run with
// gmsh -2 -format stl -o take5.stl annular_structured.geo
// gmsh -2 -format msh4 -o take5.msh annular_structured.geo

// define properties that we can use in gmsh-reader
// WHEN YOU TURN THESE ON, STL OUTPUT BREAKS
Physical Curve("wall") = {1:4};
Physical Curve("open") = {5:9};

//Physical Point("kutta") = {59, 29, 81};
//Physical Curve("wall") = {59, 29, 81};
//Physical Surface("inlet") = {47};
//Physical Volume("volume") = {2, 1, 3};
