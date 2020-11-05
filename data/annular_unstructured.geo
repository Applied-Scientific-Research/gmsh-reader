// -----------------------------------------------------------------------------
//
//  From Gmsh GEO tutorial 4
//
//  Built-in functions, holes in surfaces, annotations, entity colors
//
// -----------------------------------------------------------------------------

// As usual, we start by defining some variables:

Lc = 0.05;	// resolution at the inner boundary
xc = 0.0;
yc = 0.0;
r0 = 0.5;
r1 = 1.0;
Lc1 = Lc*r1/r0;
Lc2 = Lc;

// We can use all the usual mathematical functions (note the capitalized first
// letters), plus some useful functions like Hypot(a, b) := Sqrt(a^2 + b^2):

// Then we define some points and some lines using these variables:

Point(1) = {xc,    yc,    0, Lc2};
Point(2) = {xc+r0, yc,    0, Lc2};
Point(3) = {xc,    yc+r0, 0, Lc2};
Point(4) = {xc-r0, yc,    0, Lc2};
Point(5) = {xc,    yc-r0, 0, Lc2};

Point(6) = {xc+r1, yc,    0, Lc1};
Point(7) = {xc,    yc+r1, 0, Lc1};
Point(8) = {xc-r1, yc,    0, Lc1};
Point(9) = {xc,    yc-r1, 0, Lc1};
Point(10)= {xc,    yc,    0, Lc1};

// Gmsh provides other curve primitives than straight lines: splines, B-splines,
// circle arcs, ellipse arcs, etc. Here we define a new circle arc, starting at
// point 14 and ending at point 16, with the circle's center being the point 15:

Circle(1) = {2,1,3};
Circle(2) = {3,1,4};
Circle(3) = {4,1,5};
Circle(4) = {5,1,2};

Circle(5) = {6,10,7};
Circle(6) = {7,10,8};
Circle(7) = {8,10,9};
Circle(8) = {9,10,6};

// Note that, in Gmsh, circle arcs should always be smaller than Pi. The
// OpenCASCADE geometry kernel does not have this limitation.

// We can then define additional lines and circles, as well as a new surface:

// this is the hole
Curve Loop(9) = {1, 2, 3, 4};
//Plane Surface(10) = {9};	// if you define this surface, it will be meshed

// But we still need to define the exterior surface. Since this surface has a
// hole, its definition now requires two curves loops:

Curve Loop(11) = {5, 6, 7, 8};
Plane Surface(12) = {11, 9};

// As a general rule, if a surface has N holes, it is defined by N+1 curve loops:
// the first loop defines the exterior boundary; the other loops define the
// boundaries of the holes.

// This post-processing view is in the "parsed" format, i.e. it is interpreted
// using the same parser as the `.geo' file. For large post-processing datasets,
// that contain actual field values defined on a mesh, you should use the MSH
// file format instead, which allows to efficiently store continuous or
// discontinuous scalar, vector and tensor fields, or arbitrary polynomial
// order.

// define properties that we can use in gmsh-reader
//Physical Surface("symmetryLine") = {51, 37, 73};
//Physical Surface("frontAndBack") = {60, 38, 82, 16, 14, 12};
//Physical Surface("wall") = {59, 29, 81};
//Physical Surface("inlet") = {47};
//Physical Surface("outlet") = {77};
//Physical Volume("volume") = {2, 1, 3};

