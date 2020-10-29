// main.cpp
//
// (c)2020 Applied Scientific Research, Inc.
//         Mohammad Hajit

#include "read_MSH_Mesh.h"

#include <iostream>
#include <string>


int main(int argc, char const *argv[]) {

	// check command line for file name
	if (argc == 2) {
		std::string input_msh_file_name = argv[1];
		Mesh mesh;
		char status = mesh.read_msh_file(input_msh_file_name.c_str());
		return 1;
	} else {
		std::cout << std::endl << "Usage:" << std::endl;
		std::cout << "  " << argv[0] << " filename.msh" << std::endl << std::endl;
		return -1;
	}
}

