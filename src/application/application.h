#pragma once

class graphics;
class window;
class input;
class network;

class application
{
 public:
   virtual void init(void) = 0;
   virtual void run(void) = 0;
   virtual void shutdown(void) = 0;
   static application* factory(void);
   
protected:
   bool m_running = false;

   graphics* m_graphics;
   input* m_input;
   network* m_network;
   
   const char* m_name = "application";
   int m_width=512, m_height=512;
   window* m_window;
};
