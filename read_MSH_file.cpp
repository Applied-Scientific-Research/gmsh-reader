// read_MSH_file.cpp
//
// (c)2020 Applied Scientific Research, Inc.
//         Mohammad Hajit

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>


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

class Mesh {
private:
	int N_nodes = 0;     //# of nodes read from msh file
	int N_elements = 0;  //# of elements read from msh file
	std::vector<node> nodes; //coordinates of the nodes
	std::vector<edge> edges;		 // types and nodes constituting edges of elements
	std::vector<element2d> elements; // types and nodes constituting elements
	std::vector<boundary> boundaries; // the boundaries of the problem, including the names and the dges that form each boundary

public:
	Mesh() {} //default constructor

	~Mesh() {}	//destructor define here later

	char read_msh_file(const char* const filename);

	int locate_in_file(std::ifstream& filestream, const std::string& searchname); //to find a searchname in the MSH file and return the file stream

};


int main() {
	Mesh mesh;
	const char input_msh_file_name[] = "tutorial4Backup.msh"; // "mesh_file.msh";
	char status = mesh.read_msh_file(input_msh_file_name);
	return 1;
}


char Mesh::read_msh_file (const char* const filename) {
	// reads a msh file output from the Gmsh software. The msh file is in ASCII 4.1 version of the Gmsh output
	std::cout << "     Gmsh file   ***** " << filename << " *****   opened for reading ..." << std::endl << std::endl;
	int retval = 1, tmp, tmp1, tmp2;
	unsigned int index;
	unsigned int nodes_min_index, nodes_max_index, raw_N_nodes, tag_N_nodes, nodes_total_entities, group_tag, entity_dim, unorganized_node_index = 0;
	unsigned int elements_min_index, elements_max_index, tag_N_elements, elements_total_entities, element_type;
	unsigned int N_boundary, node_index;
	unsigned int entities_N_points, entities_N_curves, entities_N_faces;
	bool found=false;
	double double_field, coorX, coorY, coorZ;
	std::string temp_string;
	std::vector<unsigned int>::iterator its;
	bool check_start;
	edge _edge;
	element2d _face;
	std::vector<Cmpnts2> unorganized_nodes_coor; //store all the raw read coordinartes of all nodes
	std::vector<unsigned int> unorganized_node_mapping; // maps the node numbers read from the msh file into the unorganized indices (helps to remove the indices gap)
	std::vector<char> unorganized_node_type; //0 for corner, 1 for edge and 2 for nodes on the faces
	std::ifstream mshfile(filename);
	if (mshfile.fail()) {
		std::cout << "Input file opening failed.\n";
		exit(1);
	}

	// *************** Now read in the Boundaries field: locate the keyword $PhysicalNames *****************
	//-----------------------------------------------------------------------------------------
	tmp = locate_in_file(mshfile, "$PhysicalNames");
	mshfile >> N_boundary;
	boundaries.resize(N_boundary);
	std::vector<unsigned int> tmp_boundary_tag(N_boundary); //temporary vector to store the boundary indices in MSH file (it is often irregular)
	for (int i = 0; i < N_boundary; ++i) {
		mshfile >> tmp >> tmp_boundary_tag[i] >> boundaries[i].name;
		boundaries[i].name.erase(boundaries[i].name.size() - 1); //get rid of the "" that are in the name field read from MSH file
		boundaries[i].name.erase(0,1);
	}

	// *************** Now read in the entities field: locate the keyword $Entities *****************
	//-----------------------------------------------------------------------------------------
	tmp = locate_in_file(mshfile, "$Entities");
	mshfile >> entities_N_points >> entities_N_curves>>entities_N_faces>>tmp; //tmp is number of vols which is 0 for 2D problem
	assert(!tmp); //double check that there is no volume in the domain, so 2D problem
	
	for (int i=0; i<entities_N_points; ++i) //skip the points
		do
			getline(mshfile, temp_string);
		while (temp_string.length() == 0);


	std::vector<unsigned int> tmp_curve_entity_tag(entities_N_curves); //temporary vector to store the curves entity tag in MSH file
	std::vector<unsigned int> tmp_curves_boundary_tag(entities_N_curves); //writes the boundry tag of curves
	for (int i = 0; i < entities_N_curves; ++i) { //read in the curves entity tag and the boundry tag
		mshfile >> tmp_curve_entity_tag[i] >> double_field >> double_field >> double_field >> double_field >> double_field >> double_field
			>> tmp >> tmp_curves_boundary_tag[i];
		getline(mshfile, temp_string); //read in the rst of data which are not needed at this point
	}

	// *************** Now read in the NODES field: locate the keyword $Nodes *****************
	//-----------------------------------------------------------------------------------------
	tmp = locate_in_file(mshfile, "$Nodes");

	mshfile >> nodes_total_entities >> raw_N_nodes >>nodes_min_index>>nodes_max_index;
	unorganized_nodes_coor.resize(raw_N_nodes);
	unorganized_node_type.resize(raw_N_nodes);
	tmp = nodes_max_index - nodes_min_index + 1;
	unorganized_node_mapping.resize(tmp);
	
	// read in all the coordinates
	for (unsigned int node_entity = 0; node_entity < nodes_total_entities; ++node_entity) {
		mshfile >> entity_dim >> group_tag; //entity_dim=0(corners), 1(edge nodes) , 2(surface nodes); group_tag: tag of group for each entity
		mshfile >> tmp >> tag_N_nodes; //tmp=0,1 means No parametric or with parametric; NNodes: number of nodes in this tag
		std::fill(unorganized_node_type.begin() + unorganized_node_index, unorganized_node_type.begin() + unorganized_node_index + tag_N_nodes, entity_dim);
		for (unsigned int node = 0; node < tag_N_nodes; ++node) { //store the indices
			mshfile >> index;
			index -= nodes_min_index; //shift all indices s.t. the indices start from zero
			unorganized_node_mapping[index] = unorganized_node_index++;	//shifting all indices, such that min_index becomes zero index
		}
		unorganized_node_index -= tag_N_nodes; //restore to read the coordinates
		for (unsigned int node = 0; node < tag_N_nodes; ++node) { //now store the coordinates
			mshfile >> unorganized_nodes_coor[unorganized_node_index].x;
			mshfile >> unorganized_nodes_coor[unorganized_node_index++].y >> coorZ;
		}
	}

	// *************** Now read in the ELEMENTS field: locate the keyword $Elements *****************
	//-----------------------------------------------------------------------------------------------
	unsigned int edge_index=0; //to store in the boundaries, the edges that form each boundary curve
	//std::vector<std::vector<unsigned int>> tmp_curves_edges_tag(entities_N_curves); //store the tags for the edges forming each curve (curve is an entity, edgeis discretized form of curves)
	tmp = locate_in_file(mshfile, "$Elements");

	mshfile >> elements_total_entities >> N_elements >> elements_min_index >> elements_max_index; //N_element= total number of nodes, edges and 2d elements, ignore the 0d elements
	assert(elements_total_entities == entities_N_points + entities_N_curves + entities_N_faces);
	for (unsigned int element_entity = 0; element_entity < elements_total_entities; ++element_entity) {
		mshfile >> entity_dim >> group_tag; ////entity_dim=0(0d), 1(1d) , 2(2d) features; group_tag: tag of entity
		mshfile >> element_type >> tag_N_elements; //1,8,26,27,28: 2-node, 3-node, 4-node, 5-node and 6-node lines; 3,10,16,36,37: 4-node, 9-node, 8-node, 16-node, 25-node 2D elements
		if (entity_dim==0 /*element_type==15*/)  //single-node point
			for (unsigned int element = 0; element < tag_N_elements; ++element) //skip the nodes definitions
				mshfile >> tmp1 >> tmp2;

		else if (entity_dim==1) { // element_type corresponds to edge
			its = std::find(edge_type_node_number[0].begin(),edge_type_node_number[0].end(), element_type);
			check_start = its != edge_type_node_number[0].end(); //true means the element is of edge type
			if (!check_start) {
				std::cout << "This edge type " << element_type << " is not supported, remesh" <<std::endl;
				return 2;
			}
			
			index = its - edge_type_node_number[0].begin();
			_edge.N_nodes = edge_type_node_number[1][index]; //number of nodes for this edge type element
			_edge.edge_type = element_type; //type of edge based on the value written in the msh file

			// in general group_tag (curve_tag here) forming edges can be written NOT in the same order as in the entity section. So, find the tmp_curve_entity_tag index that has group_tag
			its = std::find(tmp_curve_entity_tag.begin(), tmp_curve_entity_tag.end(), group_tag);
			index = its - tmp_curve_entity_tag.begin();  //index of the curve_tag (group_tag) in the vector tmp_curve_entity_tag (to find the corresponding boundary tag)

			
			int curve_boundary_tag = tmp_curves_boundary_tag[index];
			its = std::find(tmp_boundary_tag.begin(), tmp_boundary_tag.end(), curve_boundary_tag);
			index = its - tmp_boundary_tag.begin();  //index of the boundary_tag in the vector tmp_boundary_tag which corresponds to the curve_boundary_tag. it is used to extract the boundary name below
			boundaries[index].N_edges += tag_N_elements; //number of edges that from the boundary
			boundaries[index].edges.resize(boundaries[index].N_edges); //open up more space at the end of the vector, in case a boundary is made of multiple curves (and each curve has multiple edges)

			//tmp_curves_edges_tag[index].resize(tag_N_elements); //to store the edges tags for the curves (which are defined in entities)

			for (unsigned int i = boundaries[index].N_edges - tag_N_elements; i < boundaries[index].N_edges; ++i) {
				boundaries[index].edges[i] = edge_index++; // the index corresponding to edges vector
				//boundaries[index].edges.push_back(edge_index++); // the index corresponding to edges vector

				mshfile >> tmp;
				_edge.nodes.clear();
				for (unsigned int j = 0; j < _edge.N_nodes; ++j) {
					mshfile >> node_index;
					_edge.nodes.push_back(node_index- nodes_min_index);
				}
				edges.push_back(_edge);
			}
		}
		else if (entity_dim == 2) { // element_type corresponds to face
			its = std::find(face_type_node_number[0].begin(), face_type_node_number[0].end(), element_type);
			check_start = its != face_type_node_number[0].end(); //true means the element is of face type
			if (!check_start) {
				std::cout << "The element type " << element_type << " is not supported, remesh" << std::endl;
				return 3;
			}
			index = its - face_type_node_number[0].begin();
			_face.N_nodes = face_type_node_number[1][index]; //number of nodes for this face type element
			_face.element_type = element_type; //type of face based on the value written in the msh file
			for (unsigned int i = 0; i < tag_N_elements; ++i) {
				mshfile >> tmp;
				_face.nodes.clear();
				for (unsigned int j = 0; j < _face.N_nodes; ++j) {
					mshfile >> index;
					_face.nodes.push_back(index-nodes_min_index);
				}
				elements.push_back(_face);
			}
		}
	}

	// Now that the edge and elements are stored, the nodes constituting them should be renumbered
	// use only the nodes that are used in the edges and elements vectors
	node _node;
	node_index = 0;
	nodes.reserve(raw_N_nodes); //maximum possible number of nodes, some may not be used in the mesh
	std::vector<bool> used_node(raw_N_nodes, false); //to keep track of the nodes in the edges and elements that are used once
	std::vector<unsigned int> second_mapping(raw_N_nodes);
	for (unsigned int i = 0; i < edges.size(); ++i) {
		for (unsigned int j = 0; j < edges[i].N_nodes; ++j) {
			tmp = edges[i].nodes[j]; //tmp should map into node_index now
			tmp1 = unorganized_node_mapping[tmp];
			if (!used_node[tmp1]) {
				used_node[tmp1] = true;
				second_mapping[tmp1] = node_index++;
				_node.node_type = unorganized_node_type[tmp1];
				_node.coor = unorganized_nodes_coor[tmp1];
				nodes.emplace_back(_node);
			}
			edges[i].nodes[j] = second_mapping[tmp1];  //redistributing the indices
			
		}
	}
	for (unsigned int i = 0; i < elements.size(); ++i) {
		for (unsigned int j = 0; j < elements[i].N_nodes; ++j) {
			tmp = elements[i].nodes[j]; //tmp should map into index now
			tmp1 = unorganized_node_mapping[tmp];
			
			if (!used_node[tmp1]) {
				used_node[tmp1] = true;
				second_mapping[tmp1] = node_index++;
				_node.node_type = unorganized_node_type[tmp1];
				_node.coor = unorganized_nodes_coor[tmp1];
				nodes.emplace_back(_node);
			}
			elements[i].nodes[j] = second_mapping[tmp1];  //redistributing the indices
		}
	}

	std::cout << "         Done" << std::endl;
	mshfile.close();

	return retval;
}

int Mesh::locate_in_file(std::ifstream& filestream, const std::string& searchname) {
	//to find a specific keyword in the MSH file and return the file stream
	std::string temp;
	bool found = false;
	while (!filestream.eof()) {
		getline(filestream, temp);
		found = false;
		if (temp == searchname) {
			found = true;
			break;
		}
	}
	if (!found) {
		std::cout << "The  " << searchname << "  Field Not Found! " << std::endl;
		return 10;
	}
	return 0;
}
