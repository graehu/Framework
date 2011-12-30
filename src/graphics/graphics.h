#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include "../types/rect.h"
#include "iRenderVisitor.h"


class graphics
{
 public:

  virtual int init(void) = 0; //initializing gl and what not for whatever window system is implamented
  virtual int render(void) = 0; //this'll have flipping the buffers

  // Data loading
  virtual int loadImage(char* _fileName) = 0;
  virtual int unloadImage(int _imageID) = 0;

  virtual int shutdown(void) = 0; //shutsdown the graphics engine
  virtual int update(void) = 0; //this is currently useless.
  virtual iRenderVisitor* getRenderer(void) = 0; //passes back the renderer

  static graphics* graphicsFactory(void);

 protected:


 private:
};

#endif//GRAPHICS_H


  // Things that I want:
  // It should be plugable. It currently is.
  // Entities should deal with rendering themselves. They currently don't.

  //  virutal int loadModel(char* _fileName) = 0;
  //  virtual int unloadModel(int _modelID) = 0;
  // Trying to work out methods to pass control.
  // passing out a model/image control structure
  // would mean that I'd have to keep a track of
  // references to stop untimely unloading.

  // Control structure passing.
  //virtual image* getImage() = 0;
  //virtual model* getModel() = 0;

  // I don't think a visitor will work because
  // I would only want to define functionality
  // this class doesn't already have. I wouldn't
  // want to have to redefine image loading in
  // derived classes.

  // I don't think this is what I want.
  //virtual void accept() = 0;
