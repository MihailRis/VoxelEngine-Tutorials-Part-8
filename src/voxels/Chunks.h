#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>
#include <glm/glm.hpp>

using namespace glm;

class Chunk;
class voxel;

class Chunks {
public:
	Chunk** chunks;
	size_t volume;
	unsigned int w,h,d;

	Chunks(int w, int h, int d);
	~Chunks();

	Chunk* getChunk(int x, int y, int z);
	Chunk* getChunkByVoxel(int x, int y, int z);
	voxel* get(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);
	voxel* rayCast(vec3 start, vec3 dir, float maxLength, vec3& end, vec3& norm, vec3& iend);

	void write(unsigned char* dest);
	void read(unsigned char* source);
};

#endif /* VOXELS_CHUNKS_H_ */
