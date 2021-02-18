#include "mpeg_sample.h"
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "mpeg_writer.h"
#include "mpeg_reader.h"
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include "../../input/input.h"
#include "../../networking/connection/http_server.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
//STD includes
#include <chrono>
#include <thread>
#include <string>

using namespace fw;

application* application::factory()
{
   static bool do_once = true;
   if(do_once)
   {
      params::add("mpeg.port", {"8000"});
      commandline::parse();
      log::topics::add("mpeg_sample");
      do_once = false;
      return new mpeg_sample();
   }
   return nullptr;
}
static const char big_message[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin efficitur nulla ullamcorper, posuere nunc id, elementum diam. Morbi arcu tellus, scelerisque a ex sit amet, volutpat auctor erat.";


namespace net
{
   class mpeg_handler : public http_server::handler
   {
   public:
      void ws_send(http_server::handler::ws_send_callback callback) override
      {
	 if(data != nullptr && size > 0)
	 {
	    if(!sending_frames)
	    {
	       sending_frames = true;
	       // log::info("mpeg_handler send frame: {}", size);
	    }
	    // callback((const char*)data, size, false);
	    // callback("hello", sizeof("hello"), true);
	    data = nullptr;
	    size = 0;
	 }
	 else
	 {
	    if(sending_frames)
	    {
	       sending_frames = false;
	    }
	 }
      }
      bool is_ws_handler() override { return true; }
      unsigned char* data;
      int size;
   private:
      bool sending_frames = false;
   };
}

void generate_rgb(int width, int height, uint8_t **rgbp, bool on = false);

static const struct
{
   float x, y;
   float r, g, b;
} vertices[3] =
{
   { -0.6f, -0.4f, 1.f, 0.f, 0.f },
   {  0.6f, -0.4f, 0.f, 1.f, 0.f },
   {   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static const char* vertex_shader_text =
   R"(
#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec2 vPos;
varying vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";
 
static const char* fragment_shader_text =
   R"(
#version 110
varying vec3 color;
void main()
{
    gl_FragColor = vec4(color, 1.0);
}
)";

static void glfw_error_callback(int error, const char* description)
{
   (void)error;
   fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   (void)scancode;
   (void)mods;
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);       
   }
}

void mpeg_sample::run(void)
{
   log::scope topic("mpeg_sample");
   log::info("----------------");
   int port = 0;
   auto val = params::get_value("mpeg.port", 0);
   log::info("port set: {}", val);
   port = std::from_string<int>(val);
   log::debug("entering loop");
   net::http_server l_http_server(port);
   commandline::http_handler lv_params_handler;
   l_http_server.add_handler(&lv_params_handler);
   net::mpeg_handler lv_mpeg_handler;
   l_http_server.add_handler(&lv_mpeg_handler);
   
   GLFWwindow* window;
   GLuint vertex_buffer, vertex_shader, fragment_shader, program;
   GLint mvp_location, vpos_location, vcol_location;
 
   glfwSetErrorCallback(glfw_error_callback);
 
   // if (glfwInit())
   if(false)
   { 
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
      window = glfwCreateWindow(640, 480, "mpeg_sample", NULL, NULL);
      if (!window)
      {
	 glfwTerminate();
	 exit(EXIT_FAILURE);
      }
 
      glfwSetKeyCallback(window, key_callback);
   
      glfwMakeContextCurrent(window);
      gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
      glfwSwapInterval(1);
 
      // NOTE: OpenGL error checks have been omitted for brevity
 
      glGenBuffers(1, &vertex_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
      vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
      glCompileShader(vertex_shader);
 
      fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
      glCompileShader(fragment_shader);
 
      program = glCreateProgram();
      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);
      glLinkProgram(program);
 
      mvp_location = glGetUniformLocation(program, "MVP");
      vpos_location = glGetAttribLocation(program, "vPos");
      vcol_location = glGetAttribLocation(program, "vCol");
 
      glEnableVertexAttribArray(vpos_location);
      glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
			    sizeof(vertices[0]), (void*) 0);
      glEnableVertexAttribArray(vcol_location);
      glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
			    sizeof(vertices[0]), (void*) (sizeof(float) * 2));

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      mpeg_writer gl_writer("gl_mpeg", width, height, 30);
      uint8_t* data = new uint8_t[3*width*height];
      while (!glfwWindowShouldClose(window))
      {
	 float ratio;
	 mat4x4 m, p, mvp;
	 glfwGetFramebufferSize(window, &width, &height);
	 glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	 gl_writer.add_frame(data);
	 lv_mpeg_handler.data = gl_writer.pleb;
	 lv_mpeg_handler.size = gl_writer.pleb_size;
	 ratio = width / (float) height;
 
	 glViewport(0, 0, width, height);
	 glClear(GL_COLOR_BUFFER_BIT);
 
	 mat4x4_identity(m);
	 mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	 mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	 mat4x4_mul(mvp, p, m);
 
	 glUseProgram(program);
	 glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
	 glDrawArrays(GL_TRIANGLES, 0, 3);
 
	 glfwSwapBuffers(window);
	 glfwPollEvents();
      }
      delete [] data;
      glfwDestroyWindow(window);
 
      glfwTerminate();
      mpeg_reader reader("gl_mpeg.1.00.h264");
      reader.dump_screenshot(200);
   }
   else
   {
      input* l_input = input::inputFactory();
      l_input->init();
      bool l_looping = true;
      mpeg_reader reader("gl_mpeg.1.00.h264");
      while(l_looping)
      {
	 if(l_input->update()) l_looping = false;
	 if(reader.fill_packet())
	 {
	    lv_mpeg_handler.data = reader.packet_data;
	    lv_mpeg_handler.size = reader.packet_data_size;
	 }
	 else
	 {
	    lv_mpeg_handler.data = nullptr;
	    lv_mpeg_handler.size = 0;
	 }
	 std::this_thread::sleep_for(std::chrono::milliseconds(30));
      }
      delete l_input;
   }
   log::debug("ending loop");
   exit(EXIT_SUCCESS);
}
