#include "wSdlGl.h"


#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

int wSdlGl::init(int _width, int _height, char* _name)
{
	//initialize the SDL video component
	if (SDL_Init(SDL_INIT_VIDEO) != 0) 
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
	
	//When this program exits, SDL_Quit must be called
	atexit(SDL_Quit);
	//sdl window
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);	//Use at least 5 bits of Red
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);	//Use at least 5 bits of Green
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);	//Use at least 5 bits of Blue
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);	//Use at least 16 bits for the depth buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	//Enable double buffering

	m_screen = SDL_SetVideoMode(_width, _height, 16, SDL_OPENGLBLIT); //| SDL_FULLSCREEN in last argument to fullscreen
	SDL_WM_SetCaption((const char*)_name, NULL); // remember to set the icon at some point in the future.
	 
	 if (m_screen == NULL)
    printf("Unable to set video mode: %s\n", SDL_GetError());

	m_width = _width;
	m_height = _height;
	m_name = _name;
	glEnable(GL_TEXTURE_2D);

	
	//sets the backing colour and alpha
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	//multiplys the current matrix by an ortho matrix
	glOrtho(0.0f, _width, _height, 0.0f, -1.0f, 1.0f);
	//
	glViewport(0,0,_width,_height);


	//load projection matrix mode... inorder to use gluPerspective which sets up a perspective projection matrix >.>
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//calculate aspect ratio
	//TODO: find something to replace this call. It's the only use of Glut.
	gluPerspective(45.0f,(GLfloat)_width/(GLfloat)_height, 1 ,4000.0f);

	glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
	glLoadIdentity();// Reset The Modelview Matrix
	
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_CULL_FACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);//*/
  return 0;
}

int wSdlGl::move(int _x, int _y)
{
  return 0;
}

int wSdlGl::resize(int _width, int _height)
{
  return 0;
}


window* window::windowFactory()
{
  return (window*)new wSdlGl;
}
