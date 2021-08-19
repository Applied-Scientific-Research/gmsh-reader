// -----------------------------------------------------------------------------
//
//  Structured straight channel mesh
//
// -----------------------------------------------------------------------------

xs = 0.0;
ys = 0.0;
xf = LEN;
yf = HGT;
tk = THK;
cell = CELL;

// define some points and some lines using these variables:

Point(1) = {xs,    ys,    0};
Point(2) = {xs+tk, ys,    0};
Point(3) = {xf-tk, ys,    0};
Point(4) = {xf,    ys,    0};

Point(5) = {xs,    ys+tk, 0};
Point(6) = {xs+tk, ys+tk, 0};
Point(7) = {xf-tk, ys+tk, 0};
Point(8) = {xf,    ys+tk, 0};

Point(9) = {xs,    yf-tk, 0};
Point(10) = {xs+tk, yf-tk, 0};
Point(11) = {xf-tk, yf-tk, 0};
Point(12) = {xf,    yf-tk, 0};

Point(13) = {xs,    yf,    0};
Point(14) = {xs+tk, yf,    0};
Point(15) = {xf-tk, yf,    0};
Point(16) = {xf,    yf,    0};

// these are the long edges of blocks
Line(1) = {2,3};
Line(2) = {7,6};
Line(3) = {8,12};
Line(4) = {11,7};
Line(5) = {15,14};
Line(6) = {10,11};
Line(7) = {9,5};
Line(8) = {6,10};
Transfinite Curve{1:2} = (xf-2*tk)/cell + 1;
Transfinite Curve{5:6} = (xf-2*tk)/cell + 1;
Transfinite Curve{3:4} = (yf-2*tk)/cell + 1;
Transfinite Curve{7:8} = (yf-2*tk)/cell + 1;

// these are the short edges, going from thin to thick
Line(9)  = {1,2};
Line(10) = {2,6};
Line(11) = {5,1};
Line(12) = {5,6};
Line(13) = {4,8};
Line(14) = {8,7};
Line(15) = {3,4};
Line(16) = {3,7};
Line(17) = {16,15};
Line(18) = {15,11};
Line(19) = {12,16};
Line(20) = {12,11};
Line(21) = {13,9};
Line(22) = {9,10};
Line(23) = {14,13};
Line(24) = {14,10};
Transfinite Curve{9,10,12,13,14,16,17,18,20,21,22,24} = NPTS Using Progression PROG;
Transfinite Curve{11,15,19,23} = NPTS Using Progression 1.0/PROG;

// the long edge blocks
Curve Loop(25) = {1, 16, 2, -10};
Plane Surface(26) = {25};
Transfinite Surface{26};
Recombine Surface{26};

Curve Loop(27) = {-14, 3, 20, 4};
Plane Surface(28) = {27};
Transfinite Surface{28};
Recombine Surface{28};

Curve Loop(29) = {6, -18, 5, 24};
Plane Surface(30) = {29};
Transfinite Surface{30};
Recombine Surface{30};

Curve Loop(31) = {12, 8, -22, 7};
Plane Surface(32) = {31};
Transfinite Surface{32};
Recombine Surface{32};

// the corner blocks
Curve Loop(33) = {9, 10, -12, 11};
Plane Surface(34) = {33};
Transfinite Surface{34};
Recombine Surface{34};

Curve Loop(35) = {15, 13, 14, -16};
Plane Surface(36) = {35};
Transfinite Surface{36};
Recombine Surface{36};

Curve Loop(37) = {-20, 19, 17, 18};
Plane Surface(38) = {37};
Transfinite Surface{38};
Recombine Surface{38};

Curve Loop(39) = {22, -24, 23, 21};
Plane Surface(40) = {39};
Transfinite Surface{40};
Recombine Surface{40};

// set mesh geometry order here
Mesh.ElementOrder = ORD;

// always this (i.e. always use actual geometry for the 2nd and higher order nodes
Mesh.SecondOrderLinear = 0;

// run with
// gmsh -2 -format stl -o take5.stl annular_structured.geo
// gmsh -2 -format msh4 -o take5.msh annular_structured.geo

// define properties that we can use in gmsh-reader
// WHEN YOU TURN THESE ON, STL OUTPUT BREAKS
// IF YOU DONT TURN THESE ON, ELEMENTS WILL NOT BE WRITTEN TO MSH

// in 2D, Curve is a BC and Surface is the solution domain
Physical Curve("inlet") = {21,7,11};
Physical Curve("outlet") = {13,3,19};
Physical Curve("wall") = {9,1,15,17,5,23};
Physical Curve("open") = {2,8,6,4};
Physical Surface("fluid") = {26,28,30,32,34,36,38,40};

// in 3D, Curve is an edge, Surface is a BC, and Volume is the solution domain
//Physical Point("kutta") = {59, 29, 81};
//Physical Curve("wall") = {59, 29, 81};
//Physical Volume("fluid") = {12, 14, 16, 18};

