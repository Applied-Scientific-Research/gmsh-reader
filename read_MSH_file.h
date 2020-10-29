// read_MSH_file.cpp
//
// (c)2020 Applied Scientific Research, Inc.
//         Mohammad Hajit

#include <cmath>
#include <string>
#include <vector>

//enum edges_types {_2Node_edge = 1, _3Node_edge = 8, _4Node_edge = 26, _5Node_edge = 27, _6Node_edge = 28};
std::vector< std::vector<unsigned int>> edge_type_node_number{ {1,8,26,27,28}, {2,3,4,5,6} }; //holds the number of nodes on each edge type
std::vector< std::vector<unsigned int>> face_type_node_number{ {3,10,16,36,37}, {4,9,8,16,25} }; //holds the number of nodes on each 2D element type

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
	unsigned int element_type;
	unsigned int N_nodes;
	std::vector<unsigned int> nodes;
};
struct edge {  //the side edges of the 2D elements
	unsigned int edge_type;
	unsigned int N_nodes;
	std::vector<unsigned int> nodes;
};
struct node {  //the nodes
	unsigned int node_type;  //0 for corner, 1 for nodes on the edges and 2 for nodes on the faces
	Cmpnts2 coor;
};
struct boundary {  //the boundary for 2D mesh
	std::string name;  //name of the boundary
	unsigned int N_edges; //number of the 1D edges constituting the boundary
	std::vector<unsigned int> edges;  //the index of the edges that form the boundary
	boundary() : N_edges(0){} //default constructor
};

