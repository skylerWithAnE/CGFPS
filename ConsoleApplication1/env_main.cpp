
#include "glfuncs.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <stdexcept>
#include <cassert>
#include <stdlib.h>
#include <ciso646>
#include <typeinfo>
#include <cmath>
#include <iostream>

using namespace std;

#include <SDL.h>
#include "Program.h"
#include "math3d.h"
#include "ImageCubeTexture.h"
#include "Camera.h"
#include "Mesh.h"

using namespace std;

void APIENTRY debugcallback(GLenum source, GLenum typ,
	GLuint id_, GLenum severity, GLsizei length,
	const GLchar* message, const void* obj) {

	cout << message << endl; ;
	if (source != GL_DEBUG_SOURCE_SHADER_COMPILER && severity == GL_DEBUG_SEVERITY_HIGH) {
		throw runtime_error("GL error");
	}
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* win = SDL_CreateWindow(
		"ETGG",
		20, 20,
		512, 512,
		SDL_WINDOW_OPENGL);
	if (!win) {
		throw runtime_error("Could not create window");
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GLContext rc = SDL_GL_CreateContext(win);
	if (!rc) {
		throw runtime_error("Cannot create GL context");
	}

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, 1);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugcallback, NULL);


	Program* prog = new Program("vs.txt", "fs.txt");
	prog->use();

	Camera* cam = new Camera();
	cam->look_at(vec3(2, 1, 2), vec3(0, 1, 0), vec3(0, 1, 0));
	Mesh* meshes[] = {
		new Mesh("robot.obj.mesh"),
		new Mesh("bumpyrobot.obj.mesh"),
		new Mesh("sword.obj.mesh")
	};
	int whichmesh = 0;
	bool reflective = true;

	//for cubemaps rendered from Blender:
	//y and z are swapped compared to how GL thinks of them.
	ImageCubeTexture* T = new ImageCubeTexture(
		//{"s3px.png","s3nx.png","s3pz.png","s3nz.png","s3py.png","s3ny.png"} );
		//{"tpx.png","tnx.png","tpy.png","tny.png","tpz.png","tnz.png"} );
	{ "px.png","nx.png","pz.png","nz.png","py.png","ny.png" }
		//{   "assets/xp.png","assets/xm.png",
		//    "assets/zp.png","assets/zm.png",
		//    "assets/yp.png","assets/ym.png"
		//}
	);

	glClearColor(0.2, 0.4, 0.6, 1.0);
	glEnable(GL_DEPTH_TEST);

	set<int> keys;

	int last = SDL_GetTicks();
	SDL_Event ev;


	while (1) {
		while (1) {
			if (!SDL_PollEvent(&ev)) {
				break;
			}

			if (ev.type == SDL_QUIT) {
				exit(0);
			}
			else if (ev.type == SDL_KEYDOWN) {
				int k = ev.key.keysym.sym;
				keys.insert(k);
				if (k == SDLK_q)
					exit(0);
				else if (k == SDLK_F1)
					whichmesh = (whichmesh + 1) % 3;
				else if (k == SDLK_F2)
					reflective = !reflective;
			}
			else if (ev.type == SDL_KEYUP) {
				int k = ev.key.keysym.sym;
				keys.erase(k);
			}
			else if (ev.type == SDL_MOUSEBUTTONDOWN) {
				//print("mouse down:",ev.button.button,ev.button.x,ev.button.y);
			}
			else if (ev.type == SDL_MOUSEBUTTONUP) {
				//cout << ("mouse up:",ev.button.button,ev.button.x,ev.button.y);
			}
			else if (ev.type == SDL_MOUSEMOTION) {
				//int dx = ev.motion.xrel;
				//cam->turn(-0.01*dx);
			}
		}

		int now = SDL_GetTicks();
		float elapsed = now - last;
		last = now;

		if (keys.find(SDLK_w) != keys.end())
			cam->walk(0.001*elapsed);
		if (keys.find(SDLK_s) != keys.end())
			cam->walk(-0.001*elapsed);
		if (keys.find(SDLK_a) != keys.end())
			cam->turn(0.001*elapsed);
		if (keys.find(SDLK_d) != keys.end())
			cam->turn(-0.001*elapsed);

		if (keys.find(SDLK_o) != keys.end())
			cam->tilt(0.001*elapsed);
		if (keys.find(SDLK_p) != keys.end())
			cam->tilt(-0.001*elapsed);


		if (keys.find(SDLK_i) != keys.end())
			cam->strafe(0, 0.001*elapsed, 0);
		if (keys.find(SDLK_k) != keys.end())
			cam->strafe(0, -0.001*elapsed, 0);
		if (keys.find(SDLK_j) != keys.end())
			cam->strafe(-0.001*elapsed, 0, 0);
		if (keys.find(SDLK_l) != keys.end())
			cam->strafe(0.001*elapsed, 0, 0);

		prog->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cam->draw(prog);
		prog->setUniform("lightPos", cam->eye.xyz());
		prog->setUniform("worldMatrix", mat4::identity());
		prog->setUniform("skybox", T);
		prog->setUniform("reflective", reflective ? 1.0 : 0.0);
		meshes[whichmesh]->draw(prog);



		SDL_GL_SwapWindow(win);
	} //endwhile

	return 0;
}


Program* Program::active = NULL;