#include "wWinGl.h"
#include <windows.h>
#include <iostream>


#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/glut.h>

#include <GL/gl.h>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glut.lib")
#pragma comment(lib, "glut32.lib")



int wWinGl::resize(int _width, int _height)
{
	return 0;
}

int wWinGl::move(int _x, int _y)
{
	return 0;
}
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
///this assumes that the function will be defined at some point in the near future.

int wWinGl::init(int _width, int _height, char *_name)
{

	HDC hdc;
	HGLRC hrc;

	m_width = _width;
	m_height = _height;

	HWND wHwnd;
	WNDCLASSEX wcex;

	wcex.cbSize        = sizeof (wcex);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = GetModuleHandle(0);
	wcex.hIcon         = 0;
	wcex.hCursor       = LoadCursor (NULL, IDC_ARROW);

	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = "FirstWindowClass";
	wcex.hIconSm       = 0;

	RegisterClassEx (&wcex);// Register the class

	//Create the window
	wHwnd = CreateWindow ("FirstWindowClass",
						_name,
						WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						_width,
						_height,
						NULL,
						NULL,
						GetModuleHandle(0),
						NULL
					);

	if(!wHwnd) return NULL;// If we couldn't get a handle, return NULL
	SetWindowLongPtr(wHwnd, GWLP_USERDATA, (long)this);

	ShowWindow (wHwnd, true);//winManager::instance().getNCmdShow());
	UpdateWindow (wHwnd);	

	hdc = GetDC(wHwnd); // Get the device context for our window

	PIXELFORMATDESCRIPTOR pfd; // Create a new PIXELFORMATDESCRIPTOR (PFD)
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)); // Clear our  PFD
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // Set the size of the PFD to the size of the class
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW; // Enable double buffering, opengl support and drawing to a window
	pfd.iPixelType = PFD_TYPE_RGBA; // Set our application to use RGBA pixels
	pfd.cColorBits = 32; // Give us 32 bits of color information (the higher, the more colors)
	pfd.cDepthBits = 32; // Give us 32 bits of depth information (the higher, the more depth levels)
	pfd.iLayerType = PFD_MAIN_PLANE; // Set the layer of the PFD

	int nPixelFormat = ChoosePixelFormat(hdc, &pfd); // Check if our PFD is valid and get a pixel format back
	if (nPixelFormat == 0) // If it fails
			return false;

	bool bResult = SetPixelFormat(hdc, nPixelFormat, &pfd); // Try and set the pixel format based on our PFD
	if (!bResult) // If it fails
		return false;

	HGLRC tempOpenGLContext = wglCreateContext(hdc); // Create an OpenGL 2.1 context for our device context
	wglMakeCurrent(hdc, tempOpenGLContext); // Make the OpenGL 2.1 context current and active

	GLenum error = glewInit(); // Enable GLEW
	if (error != GLEW_OK) // If GLEW fails
		return false;

	int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, // Set the MAJOR version of OpenGL to 3
		WGL_CONTEXT_MINOR_VERSION_ARB, 2, // Set the MINOR version of OpenGL to 2
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // Set our OpenGL context to be forward compatible
		0
	};

	if (wglewIsSupported("WGL_ARB_create_context") == 1) { // If the OpenGL 3.x context creation extension is available
		hrc = wglCreateContextAttribsARB(hdc, NULL, attributes); // Create and OpenGL 3.x context based on the given attributes
		wglMakeCurrent(NULL, NULL); // Remove the temporary context from being active
		wglDeleteContext(tempOpenGLContext); // Delete the temporary OpenGL 2.1 context
		wglMakeCurrent(hdc, hrc); // Make our OpenGL 3.0 context current
	}
	else {
		hrc = tempOpenGLContext; // If we didn't have support for OpenGL 3.x and up, use the OpenGL 2.1 context
	}

	const GLubyte *glVersionString = glGetString(GL_VERSION); // Get the version of OpenGL we are using
	int glVersion[2] = {-1, -1}; // Set some default values for the version
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); // Get back the OpenGL MAJOR version we are using
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); // Get back the OpenGL MAJOR version we are using

	std::cout << "Using OpenGL: " << glVersion[0] << "." << glVersion[1] << std::endl; // Output which version of OpenGL we are using

	/////
	
	glClearColor(0.5, 0.5, 0.5, 1);
	//shader = new gShader("test.vs", "test.gs", "test.fs"); // Create our shader by loading our vertex and fragment shader
	//projectionMatrix.Perspective(60.0f, (float)m_width/(float)m_height, 0.1f, 100.f);// = glm::perspective(60.0f, (float)wWidth / (float)wHeight, 0.1f, 100.f);  // Create our perspective matrix
	//viewMatrix.Translate(0,0,-100);

	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		throw -1;
	}

	return 0;
	return 0;
}

window* window::windowFactory()
{
	return new wWinGl;
}