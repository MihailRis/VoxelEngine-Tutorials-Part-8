#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Mesh.h"
#include "graphics/VoxelRenderer.h"
#include "graphics/LineBatch.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "loaders/png_loading.h"
#include "voxels/voxel.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/Block.h"
#include "files/files.h"
#include "lighting/LightSolver.h"
#include "lighting/Lightmap.h"
#include "lighting/Lighting.h"

int WIDTH = 1280;
int HEIGHT = 720;

float vertices[] = {
		// x    y
	   -0.01f,-0.01f,
	    0.01f, 0.01f,

	   -0.01f, 0.01f,
	    0.01f,-0.01f,
};

int attrs[] = {
		2,  0 //null terminator
};

int main() {
	Window::initialize(WIDTH, HEIGHT, "Window 2.0");
	Events::initialize();

	Shader* shader = load_shader("res/main.glslv", "res/main.glslf");
	if (shader == nullptr){
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}

	Shader* crosshairShader = load_shader("res/crosshair.glslv", "res/crosshair.glslf");
	if (crosshairShader == nullptr){
		std::cerr << "failed to load crosshair shader" << std::endl;
		Window::terminate();
		return 1;
	}

	Shader* linesShader = load_shader("res/lines.glslv", "res/lines.glslf");
	if (linesShader == nullptr){
		std::cerr << "failed to load lines shader" << std::endl;
		Window::terminate();
		return 1;
	}

	Texture* texture = load_texture("res/block.png");
	if (texture == nullptr){
		std::cerr << "failed to load texture" << std::endl;
		delete shader;
		Window::terminate();
		return 1;
	}

	{
		// AIR
		Block* block = new Block(0,0);
		block->drawGroup = 1;
		block->lightPassing = true;
		Block::blocks[block->id] = block;

		// STONE
		block = new Block(1,2);
		Block::blocks[block->id] = block;

		// GRASS
		block = new Block(2,4);
		block->textureFaces[2] = 2;
		block->textureFaces[3] = 1;
		Block::blocks[block->id] = block;

		// LAMP
		block = new Block(3,3);
		block->emission[0] = 10;
		block->emission[1] = 0;
		block->emission[2] = 0;
		Block::blocks[block->id] = block;

		// GLASS
		block = new Block(4,5);
		block->drawGroup = 2;
		block->lightPassing = true;
		Block::blocks[block->id] = block;

		// GLASS
		block = new Block(5,6);
		Block::blocks[block->id] = block;
	}

	Chunks* chunks = new Chunks(16,16,16);
	Mesh** meshes = new Mesh*[chunks->volume];
	for (size_t i = 0; i < chunks->volume; i++)
		meshes[i] = nullptr;
	VoxelRenderer renderer(1024*1024*8);
	LineBatch* lineBatch = new LineBatch(4096);

	Lighting::initialize(chunks);

	glClearColor(0.0f,0.0f,0.0f,1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Mesh* crosshair = new Mesh(vertices, 4, attrs);
	Camera* camera = new Camera(vec3(96,16,96), radians(90.0f));

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	float camX = 0.0f;
	float camY = 0.0f;

	float speed = 15;

	int choosenBlock = 1;

	Lighting::onWorldLoaded();

	while (!Window::isShouldClose()){
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		if (Events::jpressed(GLFW_KEY_ESCAPE)){
			Window::setShouldClose(true);
		}
		if (Events::jpressed(GLFW_KEY_TAB)){
			Events::toogleCursor();
		}

		for (int i = 1; i < 6; i++){
			if (Events::jpressed(GLFW_KEY_0+i)){
				choosenBlock = i;
			}
		}
		if (Events::jpressed(GLFW_KEY_F1)){
			unsigned char* buffer = new unsigned char[chunks->volume * CHUNK_VOL];
			chunks->write(buffer);
			write_binary_file("world.bin", (const char*)buffer, chunks->volume * CHUNK_VOL);
			delete[] buffer;
			std::cout << "world saved in " << (chunks->volume * CHUNK_VOL) << " bytes" << std::endl;
		}
		if (Events::jpressed(GLFW_KEY_F2)){
			unsigned char* buffer = new unsigned char[chunks->volume * CHUNK_VOL];
			read_binary_file("world.bin", (char*)buffer, chunks->volume * CHUNK_VOL);
			chunks->read(buffer);
			delete[] buffer;

			Lighting::clear();
			Lighting::onWorldLoaded();
		}

		if (Events::pressed(GLFW_KEY_W)){
			camera->position += camera->front * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_S)){
			camera->position -= camera->front * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_D)){
			camera->position += camera->right * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_A)){
			camera->position -= camera->right * delta * speed;
		}

		if (Events::_cursor_locked){
			camY += -Events::deltaY / Window::height * 2;
			camX += -Events::deltaX / Window::height * 2;

			if (camY < -radians(89.0f)){
				camY = -radians(89.0f);
			}
			if (camY > radians(89.0f)){
				camY = radians(89.0f);
			}

			camera->rotation = mat4(1.0f);
			camera->rotate(camY, camX, 0);
		}

		{
			vec3 end;
			vec3 norm;
			vec3 iend;
			voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, end, norm, iend);
			if (vox != nullptr){
				lineBatch->box(iend.x+0.5f, iend.y+0.5f, iend.z+0.5f, 1.005f,1.005f,1.005f, 0,0,0,0.5f);

				if (Events::jclicked(GLFW_MOUSE_BUTTON_1)){
					int x = (int)iend.x;
					int y = (int)iend.y;
					int z = (int)iend.z;
					chunks->set(x,y,z, 0);
					Lighting::onBlockSet(x,y,z,0);
				}
				if (Events::jclicked(GLFW_MOUSE_BUTTON_2)){
					int x = (int)(iend.x)+(int)(norm.x);
					int y = (int)(iend.y)+(int)(norm.y);
					int z = (int)(iend.z)+(int)(norm.z);
					chunks->set(x, y, z, choosenBlock);
					Lighting::onBlockSet(x,y,z, choosenBlock);
				}
			}
		}

		Chunk* closes[27];
		for (size_t i = 0; i < chunks->volume; i++){
			Chunk* chunk = chunks->chunks[i];
			if (!chunk->modified)
				continue;
			chunk->modified = false;
			if (meshes[i] != nullptr)
				delete meshes[i];

			for (int i = 0; i < 27; i++)
				closes[i] = nullptr;
			for (size_t j = 0; j < chunks->volume; j++){
				Chunk* other = chunks->chunks[j];

				int ox = other->x - chunk->x;
				int oy = other->y - chunk->y;
				int oz = other->z - chunk->z;

				if (abs(ox) > 1 || abs(oy) > 1 || abs(oz) > 1)
					continue;

				ox += 1;
				oy += 1;
				oz += 1;
				closes[(oy * 3 + oz) * 3 + ox] = other;
			}
			Mesh* mesh = renderer.render(chunk, (const Chunk**)closes);
			meshes[i] = mesh;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw VAO
		shader->use();
		shader->uniformMatrix("projview", camera->getProjection()*camera->getView());
		texture->bind();
		mat4 model(1.0f);
		for (size_t i = 0; i < chunks->volume; i++){
			Chunk* chunk = chunks->chunks[i];
			Mesh* mesh = meshes[i];
			model = glm::translate(mat4(1.0f), vec3(chunk->x*CHUNK_W+0.5f, chunk->y*CHUNK_H+0.5f, chunk->z*CHUNK_D+0.5f));
			shader->uniformMatrix("model", model);
			mesh->draw(GL_TRIANGLES);
		}

		crosshairShader->use();
		crosshair->draw(GL_LINES);

		linesShader->use();
		linesShader->uniformMatrix("projview", camera->getProjection()*camera->getView());
		glLineWidth(2.0f);
		lineBatch->render();

		Window::swapBuffers();
		Events::pullEvents();
	}

	Lighting::finalize();

	delete shader;
	delete texture;
	delete chunks;
	delete crosshair;
	delete crosshairShader;
	delete linesShader;
	delete lineBatch;

	Window::terminate();
	return 0;
}
