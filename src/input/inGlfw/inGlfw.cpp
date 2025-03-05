#include "inGlfw.h"
#include "../../utils/log/log.h"
#include "GLFW/glfw3.h"

namespace fwvulkan
{
extern GLFWwindow* g_window;
}

input::~input()
{
}

bool g_glfw_keys[input::e_totalKeys];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { g_glfw_keys[input::e_right] = true; }
   if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) { g_glfw_keys[input::e_left] = true; }
   if (key == GLFW_KEY_UP && action == GLFW_PRESS) { g_glfw_keys[input::e_up] = true; }
   if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) { g_glfw_keys[input::e_down] = true; }
   if (key == GLFW_KEY_Q && action == GLFW_PRESS) { g_glfw_keys[input::e_quit] = true; }

   if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) { g_glfw_keys[input::e_right] = false; }
   if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) { g_glfw_keys[input::e_left] = false; }
   if (key == GLFW_KEY_UP && action == GLFW_RELEASE) { g_glfw_keys[input::e_up] = false; }
   if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) { g_glfw_keys[input::e_down] = false; }
   if (key == GLFW_KEY_Q && action == GLFW_RELEASE) { g_glfw_keys[input::e_quit] = false; }
}
int inGlfw::init()
{
   glfwSetKeyCallback(fwvulkan::g_window, key_callback);
   return 0;
}
bool inGlfw::update(void)
{
   for(int i = 0; i < input::e_totalKeys; i++) { m_keys[i] = g_glfw_keys[i]; }
   return false;
}
bool inGlfw::isMouseClicked(mouseButtons _button)
{
   return m_mouseButtons[_button];
}
bool inGlfw::isKeyPressed(keys _key)
{
   return m_keys[_key];
}
void inGlfw::mouseDelta(float& _x, float& _y){}
input* input::inputFactory()
{
  return (input*)new inGlfw; //returns an input class of sdl type
}
