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

#define MAP_KEY(glfw_key, fw_key) \
   if (key == glfw_key && action == GLFW_PRESS) { g_glfw_keys[fw_key] = true; } \
   if (key == glfw_key && action == GLFW_RELEASE) { g_glfw_keys[fw_key] = false; }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   MAP_KEY(GLFW_KEY_RIGHT, input::e_right);
   MAP_KEY(GLFW_KEY_LEFT, input::e_left);
   MAP_KEY(GLFW_KEY_UP, input::e_up);
   MAP_KEY(GLFW_KEY_DOWN, input::e_down);
   MAP_KEY(GLFW_KEY_R, input::e_respawn);
   MAP_KEY(GLFW_KEY_Q, input::e_quit);
   MAP_KEY(GLFW_KEY_G, input::e_shademode);
   MAP_KEY(GLFW_KEY_M, input::e_nextmodel);
   MAP_KEY(GLFW_KEY_LEFT_SHIFT, input::e_shift);
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
