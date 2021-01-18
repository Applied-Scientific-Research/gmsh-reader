// -----------------------------------------------------------------------------
//
//  Structured cavity mesh
//
// -----------------------------------------------------------------------------

xs = 0.0;
ys = 0.0;
xf = XSIZ;
yf = YSIZ;
tk = THICK;

// define some points and some lines using these variables:

Point(1) = {xs,    ys,    0};
Point(2) = {xf,    ys,    0};
Point(3) = {xf,    yf,    0};
Point(4) = {xs,    yf,    0};
Point(5) = {xs+tk, ys+tk, 0};
Point(6) = {xf-tk, ys+tk, 0};
Point(7) = {xf-tk, yf-tk, 0};
Point(8) = {xs+tk, yf-tk, 0};

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};

Line(5) = {5,6};
Line(6) = {6,7};
Line(7) = {7,8};
Line(8) = {8,5};

Transfinite Curve{1:8} = NCIRC;

Line(9)  = {1,5};
Line(10) = {2,6};
Line(11) = {3,7};
Line(12) = {4,8};
Transfinite Curve{9:12} = PROGRESSION;

Curve Loop(13) = {1, 10, -5, -9};
Plane Surface(14) = {13};
Transfinite Surface{14};
Recombine Surface{14};

Curve Loop(15) = {2, 11, -6, -10};
Plane Surface(16) = {15};
Transfinite Surface{16};
Recombine Surface{16};

Curve Loop(17) = {3, 12, -7, -11};
Plane Surface(18) = {17};
Transfinite Surface{18};
Recombine Surface{18};

Curve Loop(19) = {4, 9, -8, -12};
Plane Surface(20) = {19};
Transfinite Surface{20};
Recombine Surface{20};

// set mesh geometry order here
Mesh.ElementOrder = ORDER;

// always this (i.e. always use actual geometry for the 2nd and higher order nodes
Mesh.SecondOrderLinear = 0;

// run with
// gmsh -2 -format stl -o take5.stl annular_structured.geo
// gmsh -2 -format msh4 -o take5.msh annular_structured.geo

// define properties that we can use in gmsh-reader
// WHEN YOU TURN THESE ON, STL OUTPUT BREAKS
// IF YOU DONT TURN THESE ON, ELEMENTS WILL NOT BE WRITTEN TO MSH

// in 2D, Curve is a BC and Surface is the solution domain
Physical Curve("wall") = {1:4};
Physical Curve("open") = {5:8};
Physical Surface("fluid") = {14, 16, 18, 20};

// in 3D, Curve is an edge, Surface is a BC, and Volume is the solution domain
//Physical Point("kutta") = {59, 29, 81};
//Physical Curve("wall") = {59, 29, 81};
//Physical Volume("fluid") = {12, 14, 16, 18};

