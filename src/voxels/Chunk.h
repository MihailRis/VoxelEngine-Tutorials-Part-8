#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#define CHUNK_W 16
#define CHUNK_H 16
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

class voxel;
class Lightmap;

class Chunk {
public:
	int x,y,z;
	voxel* voxels;
	Lightmap* lightmap;
	bool modified = true;
	Chunk(int x, int y, int z);
	~Chunk();
};

#endif /* VOXELS_CHUNK_H_ */
