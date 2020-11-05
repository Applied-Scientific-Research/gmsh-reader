// read_MSH_file.cpp
//
// (c)2020 Applied Scientific Research, Inc.
//         Mohammad Hajit
#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <cstdint>

namespace ReadMsh {

// class for 2D coordinate system
struct Cmpnts2 {
	double x, y; //the components for 2D coordinate system

	Cmpnts2(double _x, double _y) : x(_x), y(_y){};
	Cmpnts2(const Cmpnts2& cmp) :
		x(cmp.x), y(cmp.y)
	{}
	Cmpnts2() {
		x = y = 0.; //default constructor
	} 

	Cmpnts2& operator=(const Cmpnts2& rhs) // compound assignment (does not need to be a member,
	{     // addition of rhs to *this takes place here
		x = rhs.x;
		y = rhs.y;
		return *this; // return the result by reference
	}

	~Cmpnts2() {}	//destructor define here later

	void add(Cmpnts2 a, Cmpnts2 b) {
		x = a.x + b.x;
		y = a.y + b.y;
	}
	void subtract(Cmpnts2 a, Cmpnts2 b) {
		x = a.x - b.x;
		y = a.y - b.y;
	}
	void multiply(double r, Cmpnts2 a) {
		x = r * a.x;
		y = r * a.y;
	}
	void scale(double r) {
		x *= r;
		y *= r;
	}
	double norm2() {
		return std::sqrt(x * x + y * y);
	}
	void set_coor(double a, double b) {
		x = a; y = b;
	}
};

struct element2d { // the 2D element
	uint32_t N_nodes;
	std::vector<uint32_t> nodes;
	uint8_t element_type;
};
struct edge {  //the side edges of the 2D elements
	uint32_t N_nodes;
	std::vector<uint32_t> nodes;
	uint8_t edge_type;
};
struct node {  //the nodes
	Cmpnts2 coor;
	uint8_t node_type;  //0 for corner, 1 for nodes on the edges and 2 for nodes on the faces
};
struct boundary {  //the boundary for 2D mesh
	std::string name;  //name of the boundary
	uint32_t N_edges; //number of the 1D edges constituting the boundary
	std::vector<uint32_t> edges;  //the index of the edges that form the boundary
	boundary() : N_edges(0){} //default constructor
};

}
