#pragma once

#include "iRenderVisitor.h"
#include "../utils/hasher.h"
// data loading and unloading is done dynamically.
// just try rendering something and it'll load all the needed assests.

class graphics
{
 public:

  virtual int init(void) = 0; //initializing gl and what not for whatever window system is implemented
  virtual int render(void) = 0; //this'll have flipping the buffers
   virtual bool register_shader(fw::hash::path/*name*/, const char*/*path*/)  { return 0; } // being lazy here, don't want to have to implement this for everything
  virtual int shutdown(void) = 0; //shutsdown the graphics engine
  virtual int update(void) = 0; //this is currently useless.
  virtual iRenderVisitor* getRenderer(void) = 0; //passes back the renderer

  static graphics* graphicsFactory(void);

};
