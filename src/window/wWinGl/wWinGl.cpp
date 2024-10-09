#include "wWinGl.h"
#include <windows.h>
#include <iostream>

#include <GL/gl.h>


#pragma comment(lib, "opengl32.lib")

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

	HWND hwnd;
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
	hwnd = CreateWindow ("FirstWindowClass",
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

	if(!hwnd) return NULL;// If we couldn't get a handle, return NULL
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (long)this);

	ShowWindow (hwnd, true);//winManager::instance().getNCmdShow());
	UpdateWindow (hwnd);	

	hdc = GetDC(hwnd); // Get the device context for our window


	// set the pixel format for the DC
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24; //32?
	pfd.cDepthBits = 16; //32?
	pfd.iLayerType = PFD_MAIN_PLANE;
	int format = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, format, &pfd);

	// create the render context (RC)
	hrc = wglCreateContext(hdc);

	// make it the current render context
	wglMakeCurrent(hdc, hrc);

	return 0;
}

window* window::windowFactory()
{
	return new wWinGl;
}
