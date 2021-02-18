#include "files.h"

#include <fstream>
#include <iostream>

bool write_binary_file(std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open())
		return false;
	output.write(data, size);
	output.close();
	return true;
}


bool read_binary_file(std::string filename, char* data, size_t size) {
	std::ifstream output(filename, std::ios::binary);
	if (!output.is_open())
		return false;
	output.read(data, size);
	output.close();
	return true;
}
