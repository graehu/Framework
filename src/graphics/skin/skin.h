#ifndef SKIN_H
#define SKIN_H

#include "../types/rect.h"


//This class might want rotation. That should probably be delt with inside or rect? maybe.

class skin
{
 public:

  skin();
  ~skin();

 protected:

  int m_imageId;
  rect m_imageCrop;

 private:
};

#endif//SKIN_H
