#include "inWin.h"
#include <windows.h>


int inWin::init()
{return 0;}

bool g_keys[input::e_totalKeys];
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
		case WM_CREATE:
			break;
		case WM_SIZE:
			break;
		case WM_KEYDOWN: //WINDOWS IS STUPID IT SENDS THE WRONG ASCII VALUES
			if(wParam == 'W')
				g_keys[input::e_up] = true;
			else if(wParam == 'A') //a
				g_keys[input::e_left] = true;
			else if(wParam == 'S') //s
				g_keys[input::e_down] = true;
			else if(wParam == 'D') //d
				g_keys[input::e_right] = true;
			else if(wParam == 'R') //d
				g_keys[input::e_respawn] = true;
			break;
		case WM_KEYUP:
			if(wParam == 'W') //w
				g_keys[input::e_up] = false;
			else if(wParam == 'A') //a
				g_keys[input::e_left] = false;
			else if(wParam == 'S') //s
				g_keys[input::e_down] = false;
			else if(wParam == 'D') //d
				g_keys[input::e_right] = false;
			else if(wParam == 'R') //d
				g_keys[input::e_respawn] = false;
			break;
		case WM_MOUSEMOVE:
			break;
		case WM_PAINT:
		    break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
	//get input from here or something.
};
  
bool inWin::isKeyPressed(keys _key)
{
	for(int i = 0; i < input::e_totalKeys; i++)
		m_keys[i] = g_keys[i];

	return m_keys[_key];
}
bool inWin::isMouseClicked(mouseButtons _button)
{return false;}
void inWin::mouseDelta(float& _dx, float& _dy)
{return;}

bool inWin::update()
{
	MSG msg;

	if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
           if (msg.message==WM_QUIT)
           {

			   return true;
			   //this got annoying.
               //MessageBox(NULL,"program closed correctly","herro",MB_OK);
           }

           TranslateMessage (&msg);
           DispatchMessage (&msg);
	}
	return false;
}

input* input::inputFactory(void)
{
	return new inWin;
}