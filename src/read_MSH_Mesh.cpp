// read_MSH_Mesh.h
//
// (c)2020 Applied Scientific Research, Inc.
//         Mohammad Hajit

#include "read_MSH_Mesh.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstdint>

namespace ReadMsh {

//enum edges_types {_2Node_edge = 1, _3Node_edge = 8, _4Node_edge = 26, _5Node_edge = 27, _6Node_edge = 28};
std::vector< std::vector<uint32_t>> edge_type_node_number{ {1,8,26,27,28}, {2,3,4,5,6} }; //holds the number of nodes on each edge type
std::vector< std::vector<uint32_t>> face_type_node_number{ {3,10,16,36,37}, {4,9,8,16,25} }; //holds the number of nodes on each 2D element type

//
// Non-class helper function for line-reading
// Because getline works only on text files from the OS that it was compiled on...
//
// From: https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
//
std::istream& safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

// useful for grabbing only "wall" or "open" boundary
const boundary Mesh::get_bdry(const std::string _name) {
	for (boundary thisb : boundaries) {
		if (thisb.name.compare(_name) == 0) {
			return thisb;
		}
	}
	return boundary();
}

int32_t Mesh::read_msh_file (const char* const filename) {
	// reads a msh file output from the Gmsh software. The msh file is in ASCII 4.1 version of the Gmsh output
	std::cout << "    Gmsh file (" << filename << ") opened for reading ..." << std::endl;
	int32_t retval = 1, tmp, tmp1, tmp2;
	uint32_t index;
	uint32_t nodes_min_index, nodes_max_index, raw_N_nodes, tag_N_nodes, nodes_total_entities, group_tag, entity_dim, unorganized_node_index = 0;
	uint32_t elements_min_index, elements_max_index, tag_N_elements, elements_total_entities;
	uint32_t element_type, N_boundary, node_index;
	uint32_t entities_N_points, entities_N_curves, entities_N_faces;
	double double_field;
	double coorZ = 0.0;
	std::string temp_string;
	std::vector<uint32_t>::iterator its;
	bool check_start, found;
	edge _edge;
	element2d _face;
	std::vector<Cmpnts2> unorganized_nodes_coor; //store all the raw read coordinartes of all nodes
	std::vector<uint32_t> unorganized_node_mapping; // maps the node numbers read from the msh file into the unorganized indices (helps to remove the indices gap)
	std::vector<uint8_t> unorganized_node_type; //0 for corner, 1 for edge and 2 for nodes on the faces
	std::ifstream mshfile(filename);
	if (mshfile.fail()) {
		std::cout << "Input file opening failed.\n";
		exit(1);
	}

	// *************** Check version number for match: locate the keyword $MeshFormat *****************
	//-----------------------------------------------------------------------------------------
	found = locate_in_file(mshfile, "$MeshFormat");
	if (found) {
		double version;
		int filetype, datasize;
		mshfile >> version >> filetype >> datasize;
		std::cout << "    version is " << version << " and type is " << (filetype ? "binary" : "ASCII") << std::endl;
		assert(version>=4.1 && "Error: Cannot read file versions less than 4.1");
		assert(filetype==0 && "Error: Cannot read binary format files");
	} else {
		return 10;
	}

	// *************** Now read in the Boundaries field: locate the keyword $PhysicalNames *****************
	//-----------------------------------------------------------------------------------------
	found = locate_in_file(mshfile, "$PhysicalNames");
	if (!found) return 11;

	mshfile >> N_boundary;
	boundaries.resize(N_boundary);
	std::vector<uint32_t> tmp_boundary_tag(N_boundary); //temporary vector to store the boundary indices in MSH file (it is often irregular)
	for (size_t i = 0; i < N_boundary; ++i) {
		mshfile >> tmp >> tmp_boundary_tag[i] >> boundaries[i].name;
		boundaries[i].name.erase(boundaries[i].name.size() - 1); //get rid of the "" that are in the name field read from MSH file
		boundaries[i].name.erase(0,1);
	}

	// *************** Now read in the entities field: locate the keyword $Entities *****************
	//-----------------------------------------------------------------------------------------
	found = locate_in_file(mshfile, "$Entities");
	if (!found) return 12;

	mshfile >> entities_N_points >> entities_N_curves >> entities_N_faces >> tmp; //tmp is number of vols which is 0 for 2D problem
	//std::cout << "Np " << entities_N_points << "  Nc " << entities_N_curves << "  Nf " << entities_N_faces << std::endl;

	assert(!tmp); //double check that there is no volume in the domain, so 2D problem

	// skip the points
	for (size_t i=0; i<entities_N_points; ++i) //skip the points
		do
			safeGetline(mshfile, temp_string);
		while (temp_string.length() == 0);

	// read in the curves
	std::vector<uint32_t> tmp_curve_entity_tag(entities_N_curves); //temporary vector to store the curves entity tag in MSH file
	std::vector<uint32_t> tmp_curves_boundary_tag(entities_N_curves); //writes the boundry tag of curves
	for (size_t i = 0; i < entities_N_curves; ++i) { //read in the curves entity tag and the boundry tag
		mshfile >> tmp_curve_entity_tag[i] >> double_field >> double_field >> double_field >> double_field >> double_field >> double_field
			>> tmp >> tmp_curves_boundary_tag[i];
		safeGetline(mshfile, temp_string); //read in the rst of data which are not needed at this point
	}

	// do not read in the faces

	// *************** Now read in the NODES field: locate the keyword $Nodes *****************
	//-----------------------------------------------------------------------------------------
	found = locate_in_file(mshfile, "$Nodes");
	if (!found) return 13;

	mshfile >> nodes_total_entities >> raw_N_nodes >> nodes_min_index >> nodes_max_index;
	//std::cout << "nte " << nodes_total_entities << "  Ne " << raw_N_nodes << std::endl;

	unorganized_nodes_coor.resize(raw_N_nodes);
	unorganized_node_type.resize(raw_N_nodes);
	tmp = nodes_max_index - nodes_min_index + 1;
	unorganized_node_mapping.resize(tmp);
	
	// read in all the coordinates
	for (uint32_t node_entity = 0; node_entity < nodes_total_entities; ++node_entity) {
		mshfile >> entity_dim >> group_tag; //entity_dim=0(corners), 1(edge nodes) , 2(surface nodes); group_tag: tag of group for each entity
		mshfile >> tmp >> tag_N_nodes; //tmp=0,1 means No parametric or with parametric; NNodes: number of nodes in this tag
		N_nodes += tag_N_nodes;
		std::fill(unorganized_node_type.begin() + unorganized_node_index, unorganized_node_type.begin() + unorganized_node_index + tag_N_nodes, entity_dim);
		for (uint32_t node = 0; node < tag_N_nodes; ++node) { //store the indices
			mshfile >> index;
			index -= nodes_min_index; //shift all indices s.t. the indices start from zero
			unorganized_node_mapping[index] = unorganized_node_index++;	//shifting all indices, such that min_index becomes zero index
		}
		unorganized_node_index -= tag_N_nodes; //restore to read the coordinates
		for (uint32_t node = 0; node < tag_N_nodes; ++node) { //now store the coordinates
			mshfile >> unorganized_nodes_coor[unorganized_node_index].x;
			mshfile >> unorganized_nodes_coor[unorganized_node_index++].y >> coorZ;
		}
	}

	// *************** Now read in the ELEMENTS field: locate the keyword $Elements *****************
	//-----------------------------------------------------------------------------------------------
	uint32_t edge_index=0; //to store in the boundaries, the edges that form each boundary curve
	//std::vector<std::vector<uint32_t>> tmp_curves_edges_tag(entities_N_curves); //store the tags for the edges forming each curve (curve is an entity, edgeis discretized form of curves)
	found = locate_in_file(mshfile, "$Elements");
	if (!found) return 14;

	mshfile >> elements_total_entities >> N_elements >> elements_min_index >> elements_max_index; //N_element= total number of nodes, edges and 2d elements, ignore the 0d elements
	//std::cout << "ete " << elements_total_entities << "  Ne " << N_elements << std::endl;


	for (uint32_t element_entity = 0; element_entity < elements_total_entities; ++element_entity) {

		mshfile >> entity_dim >> group_tag; ////entity_dim=0(0d), 1(1d) , 2(2d) features; group_tag: tag of entity
		mshfile >> element_type >> tag_N_elements; //1,8,26,27,28: 2-node, 3-node, 4-node, 5-node and 6-node lines; 3,10,16,36,37: 4-node, 9-node, 8-node, 16-node, 25-node 2D elements
		std::cout << "      entity " << element_entity << "  dim " << entity_dim << "  tag " << group_tag << "  type " << element_type << "  num " << tag_N_elements << std::endl;

		if (entity_dim==0 /*element_type==15*/)  //single-node point
			for (uint32_t element = 0; element < tag_N_elements; ++element) //skip the nodes definitions
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

			//std::cout << "reading tag " << index << " with " << tag_N_elements << " edges starting at " << boundaries[index].N_edges << std::endl;
			for (uint32_t i = boundaries[index].N_edges - tag_N_elements; i < boundaries[index].N_edges; ++i) {
				boundaries[index].edges[i] = edge_index++; // the index corresponding to edges vector
				//boundaries[index].edges.push_back(edge_index++); // the index corresponding to edges vector

				mshfile >> tmp;
				_edge.nodes.clear();
				//std::cout << "  edge " << edge_index << " has nodes ";
				for (uint32_t j = 0; j < _edge.N_nodes; ++j) {
					mshfile >> node_index;
					//std::cout << " " << node_index;
					_edge.nodes.push_back(node_index- nodes_min_index);
				}
				//std::cout << std::endl;
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
			for (uint32_t i = 0; i < tag_N_elements; ++i) {
				mshfile >> tmp;
				_face.nodes.clear();
				for (uint32_t j = 0; j < _face.N_nodes; ++j) {
					mshfile >> index;
					_face.nodes.push_back(index-nodes_min_index);
				}
				elements.push_back(_face);
			}
		}
	}
	mshfile.close();
	std::cout << "    read " << elements_total_entities << " total entities ..." << std::endl;

	std::cout << "    compressing node list..." << std::endl;
	// Now that the edge and elements are stored, the nodes constituting them should be renumbered
	// use only the nodes that are used in the edges and elements vectors
	node _node;
	node_index = 0;
	nodes.reserve(raw_N_nodes); //maximum possible number of nodes, some may not be used in the mesh
	std::vector<bool> used_node(raw_N_nodes, false); //to keep track of the nodes in the edges and elements that are used once
	std::vector<uint32_t> second_mapping(raw_N_nodes);
	for (size_t i = 0; i < edges.size(); ++i) {
		for (size_t j = 0; j < edges[i].N_nodes; ++j) {
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
	for (size_t i = 0; i < elements.size(); ++i) {
		for (size_t j = 0; j < elements[i].N_nodes; ++j) {
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

	// Since Gmsh does not care to order the boundary edges coherently, we must do so

	// but first, we need to ensure that all elements have their nodes ordered in CCW direction
	for (size_t i = 0; i < elements.size(); ++i) {
		//std::cout << "  checking elem " << i << std::endl;
		uint32_t num_ccw = 0;
		// HACK - ASSUMES THIS IS A QUAD
		for (size_t j = 0; j < 4; ++j) {
			const uint32_t n0 = elements[i].nodes[j];
			const uint32_t n1 = elements[i].nodes[(j+1) % 4];
			const uint32_t n2 = elements[i].nodes[(j+2) % 4];
			//std::cout << "    nodes " << n0 << ", " << n1 << ", " << n2 << std::endl;
			const struct Cmpnts2 p0 = nodes[n0].coor;
			const struct Cmpnts2 p1 = nodes[n1].coor;
			const struct Cmpnts2 p2 = nodes[n2].coor;
			const double dotprod = (p1.x-p0.x)*(p2.y-p1.y) - (p1.y-p0.y)*(p2.x-p1.x);
                        //std::cout << "    nodes " << n0 << ", " << n1 << ", " << n2 << " dot prod is " << dotprod << std::endl;
			// ALL corners must be left-turns
			if (dotprod > 0.0) ++num_ccw;
		}
		if (num_ccw == 0) {
			// flip the orientation, first the first 4 nodes
			std::reverse(elements[i].nodes.begin(),elements[i].nodes.begin()+3);
			// then the rest of the nodes
			std::reverse(elements[i].nodes.begin()+4,elements[i].nodes.end());
			std::cout << "    flipping elem " << i << std::endl;
		} else if (num_ccw == 4) {
			// all good
		} else {
			std::cout << "  elem " << i << " is not convex - errors may emerge!" << std::endl;
		}
	}

	// edge nodes are ordered so that facing the vector from nodes 0 to 1, the interior of the fluid is to the left
	std::cout << "    reordering edge nodes for coherency..." << std::endl;
	for (size_t i = 0; i < edges.size(); ++i) {
		//std::cout << "  checking edge " << i << std::endl;
		// note that no matter what order the edge is, the end nodes are always 0 and 1
		const uint32_t n0 = edges[i].nodes[0];
		const uint32_t n1 = edges[i].nodes[1];
		// find the element that contains both nodes
		for (size_t j = 0; j < elements.size(); ++j) {
			int hasnodes = 0;
			for (size_t k = 0; k < elements[j].N_nodes; ++k) {
				if (elements[j].nodes[k] == n0) ++hasnodes;
				if (elements[j].nodes[k] == n1) ++hasnodes;
			}
			if (hasnodes == 2) {
				//std::cout << "    its on elem " << j << std::endl;
				bool correctorder = false;
				// now, do the nodes appear in the correct order?
				// HACK - ASSUMES THIS IS A QUAD
				for (size_t k = 0; k < 4; ++k) {
					if (elements[j].nodes[k] == n0 &&
					    elements[j].nodes[(k+1)%4] == n1) {
						correctorder = true;
					}
				}
				if (!correctorder) {
					std::cout << "      flipping edge " << i << std::endl;
					//std::cout << "    original order ";
					//for (size_t k=0; k<edges[i].nodes.size(); ++k) std::cout << " " << edges[i].nodes[k];
					//std::cout << std::endl;
					edges[i].nodes[0] = n1;
					edges[i].nodes[1] = n0;
					if (edges[i].nodes.size() > 2) {
						// reverse the middle nodes
						std::reverse(edges[i].nodes.begin()+2, edges[i].nodes.end());
					}
					//std::cout << "    new order ";
					//for (size_t k=0; k<edges[i].nodes.size(); ++k) std::cout << " " << edges[i].nodes[k];
					//std::cout << std::endl;
				}
				// bust out of this loop
				j =  elements.size();
			}
		}
	}

	std::cout << "    done." << std::endl;

	return retval;
}

bool Mesh::locate_in_file(std::ifstream& filestream, const std::string& searchname) {
	//to find a specific keyword in the MSH file and return the file stream
	//std::cout << std::endl << "searching for (" << searchname << ")" << std::endl;

	std::string temp;
	bool found = false;

	while (!filestream.eof()) {
		safeGetline(filestream, temp);
		//std::cout << "line (" << temp << ")" << std::endl;
		found = false;
		if (temp == searchname) {
			found = true;
			break;
		}
	}

	if (!found) std::cout << "The  " << searchname << "  Field Not Found! " << std::endl;
	return found;
}

}
