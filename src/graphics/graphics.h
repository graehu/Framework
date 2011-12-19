#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include "../types/rect.h"


class graphics
{
 public:

  virtual int init() = 0; //initializing gl and what not for whatever window system is implamented
  virtual int render(void) = 0; //this'll have flipping the buffers
  virtual int loadImage(char* _fileName) = 0;
  virtual int unloadImage(int _imageID) = 0;
  virtual int blitImage(int _imageID, rect _source, rect _destination) = 0; //bliting an image to a surface. (getting ready to render)
  virtual int shutdown(void) = 0; //shutsdown the graphics engine
  virtual int update(void) = 0; //this is currently useless.

  static graphics* graphicsFactory(void);

 protected:


 private:
};

#endif//GRAPHICS_H

