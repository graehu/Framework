#include "input/input2.h"
#include "utils/log/log.h"
#include "GLFW/glfw3.h"

namespace fwvulkan
{
extern GLFWwindow* g_window;
}

bool g_glfw_keys[input2::e_totalKeys];
bool g_mouseButtons[input2::e_totalButtons];

double g_glfw_mouse_x = 0;
double g_glfw_mouse_y = 0;
double g_glfw_prev_mouse_x = 0;
double g_glfw_prev_mouse_y = 0;


#define MAP_KEY(glfw_key, fw_key) \
   if (key == glfw_key && action == GLFW_PRESS) { g_glfw_keys[fw_key] = true; } \
   if (key == glfw_key && action == GLFW_RELEASE) { g_glfw_keys[fw_key] = false; }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   (void)scancode;
   (void)mods;
   (void)window;
   MAP_KEY(GLFW_KEY_RIGHT, input2::e_right);
   MAP_KEY(GLFW_KEY_LEFT, input2::e_left);
   MAP_KEY(GLFW_KEY_UP, input2::e_up);
   MAP_KEY(GLFW_KEY_DOWN, input2::e_down);

   MAP_KEY(GLFW_KEY_D, input2::e_right);
   MAP_KEY(GLFW_KEY_A, input2::e_left);
   MAP_KEY(GLFW_KEY_W, input2::e_up);
   MAP_KEY(GLFW_KEY_S, input2::e_down);
   
   MAP_KEY(GLFW_KEY_R, input2::e_respawn);
   MAP_KEY(GLFW_KEY_Q, input2::e_quit);
   MAP_KEY(GLFW_KEY_G, input2::e_shademode);
   MAP_KEY(GLFW_KEY_M, input2::e_nextmodel);
   MAP_KEY(GLFW_KEY_LEFT_SHIFT, input2::e_shift);
}
// void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
// {
   
// }
void cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
   (void)window;
   g_glfw_prev_mouse_x = g_glfw_mouse_x;
   g_glfw_prev_mouse_y = g_glfw_mouse_y;
   
   g_glfw_mouse_x = xpos;
   g_glfw_mouse_y = ypos;
}

int input2::init()
{
   glfwSetKeyCallback(fwvulkan::g_window, key_callback);
   // glfwSetMouseButtonCallback(fwvulkan::g_window, mousebutton_callback);
   glfwSetCursorPosCallback(fwvulkan::g_window, cursor_callback);
   
   return 0;
}
bool input2::update(void)
{
   g_glfw_prev_mouse_x = g_glfw_mouse_x;
   g_glfw_prev_mouse_y = g_glfw_mouse_y;
   glfwPollEvents();
   return false;
}
bool input2::isMouseClicked(mouseButtons _button)
{
   return g_mouseButtons[_button];
}
bool input2::isKeyPressed(keys _key)
{
   return g_glfw_keys[_key];
}
bool input2::setMousePosition(float _x, float _y)
{
   glfwSetCursorPos(fwvulkan::g_window, (double)_x, (double)_y);
   g_glfw_prev_mouse_x = _x;
   g_glfw_prev_mouse_x = _y;
   g_glfw_mouse_x = g_glfw_prev_mouse_x;
   g_glfw_mouse_y = g_glfw_prev_mouse_y;
   return true;
}
bool input2::centerMousePosition()
{
   int w,h;
   glfwGetWindowSize(fwvulkan::g_window, &w, &h);
   setMousePosition(w*0.5f, h*0.5f);
   return true;
}

void input2::mouseDelta(float &_x, float &_y)
{
   _x = g_glfw_prev_mouse_x - g_glfw_mouse_x;
   _y = g_glfw_prev_mouse_y - g_glfw_mouse_y;
}
