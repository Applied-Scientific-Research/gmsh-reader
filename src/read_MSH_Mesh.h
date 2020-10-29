// read_MSH_Mesh.h
//
// (c)2020 Applied Scientific Research, Inc.
//         Mohammad Hajit
#pragma once

#include "read_MSH.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


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

