#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>

extern bool write_binary_file(std::string filename, const char* data, size_t size);
extern bool read_binary_file(std::string filename, char* data, size_t size);

#endif /* FILES_FILES_H_ */
