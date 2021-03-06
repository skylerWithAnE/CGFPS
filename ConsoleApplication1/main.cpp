
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
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

using namespace std;

#include <SDL.h>
#include "SDL_mixer.h"
#include "Program.h"
#include "math3d.h"
#include "ImageTexture.h"
#include "Camera.h"
#include "World.h"
#include "Robot.h"
#include "Square.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "SolidTexture.h"



void APIENTRY debugcallback( GLenum source, GLenum typ,
        GLuint id_, GLenum severity, GLsizei length,
        const GLchar* message, const void* obj ){
	if (id_ != 131204) 
		cout << id_ << "\n";
    if( source != GL_DEBUG_SOURCE_SHADER_COMPILER && severity == GL_DEBUG_SEVERITY_HIGH )
        throw runtime_error("Encountered very bad GL error. Stopping.");
}

int main(int argc, char* argv[])
{
	cout << "STARTING\n";

    //initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    //Bring up a window at 20,20 with size 512x512  //1300x700
    SDL_Window* win = SDL_CreateWindow( 
        "ETGG",
        20,20, 
        1300,700, 
        SDL_WINDOW_OPENGL);
    
    if(!win)
        throw runtime_error("Could not create window");

    //Request GL context for the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GLContext rc = SDL_GL_CreateContext(win);
    if(!rc)
        throw runtime_error( "Cannot create GL context" );
           
    //tell GL we want debugging messages
    glDebugMessageControl(GL_DONT_CARE,GL_DONT_CARE,GL_DONT_CARE, 0, NULL, 1 );
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugcallback,NULL);

    //initialize the audio subsystem
    Mix_Init(MIX_INIT_OGG);
    Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024 );
    Mix_AllocateChannels(16);
    Mix_Chunk* pew = Mix_LoadWAV("pew.ogg");

	Framebuffer2D* fbo1 = new Framebuffer2D(1300, 700, 2);   
	//Framebuffer2D* fbo2 = new Framebuffer2D(1300, 700, 2);   //new
	Framebuffer2D* shadowbuffer = new Framebuffer2D(512, 512, 1, GL_R32F, GL_FLOAT);
    
    World* world = new World();
	Square* usq = new Square();
	Square* usq2 = new Square();

    //shaders
	Program* prog = new Program("vs.txt", "fs.txt", { "color" });
	Program* postprocprog = new Program("ppvs.txt", "ppfs.txt", { "color" });
	Program* specprog = new Program("vss.txt", "fss.txt", { "color" });
	Program* boneprog = new Program("bonevs.txt", "bonefs.txt", { "color" });
	Program* sbprog = new Program("sbvs.txt", "sbfs.txt", {});
	//Program* shadowprog = new Program("shadowvs.txt", "shadowfs.txt", {"color"});

    //view camera
    Camera* cam = new Camera();
    cam->look_at(vec3(5,1,5), vec3(10,1,10), vec3(0,1,0) );
	Camera* lightCamera = new Camera();
	//cam->hither = 0.01f;
	lightCamera->yon = 50.f;
	lightCamera->look_at(vec3(5,1,5), vec3(10, 1, 10), vec3(0, 1, 0));
	


    //tells which keys are currently down
    set<int> keys;
    
    //for timing
    int last=SDL_GetTicks();
    SDL_Event ev;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    glClearColor(0.2f,0.4f,0.6f,1.0f);
    glEnable(GL_DEPTH_TEST);

	bool shadow_buffer_debug = false;
	float sbdcd = 1.f;
	while (1) {
		//pump the event queue...
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
				if (k == SDLK_q) {
					exit(0);
				}

			}
			else if (ev.type == SDL_KEYUP) {
				int k = ev.key.keysym.sym;
				if (k == SDLK_SPACE) {
					lightCamera->look_at(cam->eye.xyz(), cam->eye.xyz()+(-1*cam->W.xyz()), cam->V.xyz());
				}
				keys.erase(k);
			}
			else if (ev.type == SDL_MOUSEBUTTONDOWN) {
				cout << "Pew!\n";
				//-1=any channel, 0=no repeat
				Mix_PlayChannel(-1, pew, 0);
			}
			else if (ev.type == SDL_MOUSEBUTTONUP) {
				//mouse up,ev.button.button,ev.button.x,ev.button.y
			}
			else if (ev.type == SDL_MOUSEMOTION) {
				float dx = (float)ev.motion.xrel;
				float dy = (float)ev.motion.yrel;

				cam->turn(-0.01f*dx);
				cam->tilt(-0.01f*dy);
				if (shadow_buffer_debug) {
					lightCamera->turn(-0.01f*dx);
					lightCamera->tilt(-0.01f*dy);
				}
			}
		}


		int now = SDL_GetTicks();
		float elapsed = (float)(now - last);
		last = now;
		sbdcd += elapsed;

		if (keys.find(SDLK_w) != keys.end()) {
			cam->walk(0.002f*elapsed);
			//lightCamera->walk(0.002f*elapsed);
		}
		if (keys.find(SDLK_s) != keys.end()) {
			cam->walk(-0.002f*elapsed);
			//lightCamera->walk(-0.002f*elapsed);
		}
		if (keys.find(SDLK_j) != keys.end()) {
			cam->turn(0.001f*elapsed);
			//lightCamera->turn(0.001f*elapsed);
		}
		if (keys.find(SDLK_l) != keys.end()) {
			cam->turn(-0.001f*elapsed);
			//lightCamera->turn(-0.001f*elapsed);
		}
		if (keys.find(SDLK_a) != keys.end()) {
			cam->strafe(-0.002f*elapsed, 0, 0);
			//lightCamera->strafe(-0.002f*elapsed, 0, 0);
		}
		if (keys.find(SDLK_d) != keys.end()) {
			cam->strafe(0.002f*elapsed, 0, 0);
			//lightCamera->strafe(0.002f*elapsed, 0, 0);
		}
		if (keys.find(SDLK_i) != keys.end()) {
			cam->strafe(0, 0.01f*elapsed, 0);
			//lightCamera->strafe(0, 0.01f*elapsed, 0);
		}
		if (keys.find(SDLK_k) != keys.end()) {
			cam->strafe(0, -0.01f*elapsed, 0);
			//lightCamera->strafe(0, -0.01f*elapsed, 0);
		}
		if (keys.find(SDLK_SLASH) != keys.end()) {
			if (sbdcd > 200.f) {
				shadow_buffer_debug = !shadow_buffer_debug;
				sbdcd = 0.f;
			}
			
		}

		fbo1->bind();





		//***BLUR DISABLED atm
		fbo1->unbind();

		postprocprog->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		postprocprog->setUniform("tex", fbo1->texture);
		postprocprog->setUniform("numOfIterations", 2);
		usq->draw(postprocprog);

		fbo1->texture->unbind();
		

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Shadow buffer
		sbprog->use();
		if(!shadow_buffer_debug)
			shadowbuffer->bind();
		glClearColor(1.f, 1.f, 1.f, 1.f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		vec3 hyymh = vec3(lightCamera->hither, lightCamera->yon, lightCamera->yon - lightCamera->hither);
		sbprog->setUniform("hitheryon", hyymh);
		//cout << hyymh.x <<" "<< hyymh.y << " " << hyymh.z << " " << endl;
		lightCamera->draw(sbprog);
		world->draw(sbprog);
		world->robotDraw(sbprog);
		if (!shadow_buffer_debug)
			shadowbuffer->unbind();

		//Draw with shadow buffer.

		if (!shadow_buffer_debug) {
			prog->use();
			prog->setUniform("lightPos", vec3(lightCamera->eye.x, lightCamera->eye.y, lightCamera->eye.z));
			prog->setUniform("lightColor", vec4(1.f, 1.f, 1.f, 1.f));
			prog->setUniform("lightViewMatrix", lightCamera->viewmatrix);
			prog->setUniform("lightProjMatrix", lightCamera->projmatrix);
			prog->setUniform("hitheryon", hyymh);
			prog->setUniform("shadowbuffer", shadowbuffer->texture);
			prog->setUniform("magicConst", 15.0f);
			prog->setUniform("scaleFactor", 5.0f);
			cam->draw(prog);
			world->draw(prog);
			world->robotDraw(prog);
			shadowbuffer->texture->unbind();
		}

		boneprog->use();
		boneprog->setUniform("lightPos", cam->eye.xyz());
		boneprog->setUniform("roughness", 0.f);
		cam->draw(boneprog);
		world->robotUpdate(elapsed);
		world->robotDraw(boneprog);


		prog->use();
		cam->draw(prog);
		prog->setUniform("lightPos", cam->eye.xyz());
		world->draw(prog);



		GLenum err = glGetError();
		while (err != GL_NO_ERROR) {
			cout << "GL error: " << hex << err << "\n";
			err = glGetError();
		} 
        SDL_GL_SwapWindow(win);
    } //endwhile
    
    return 0;
}

//static's are defined here
Program* Program::active=NULL;
Mesh* Robot::mesh=NULL;
Texture* Texture::active_textures[128];
RenderTarget* RenderTarget::active_target;
int RenderTarget::saved_viewport[4];
int Square::vao;