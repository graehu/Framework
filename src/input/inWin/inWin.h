#ifndef INWIN_H
#define INWIN_H

#include "../input.h"


class inWin : public input
{
public:
	inWin(){};
	~inWin(){};
	int init(void); //This functions should read What keys will be assigned to the commands above.
  bool update(void);
  bool isKeyPressed(keys _key);
  bool isMouseClicked(mouseButtons _button);
  void mouseDelta(float& _dx, float& _dy);

};

#endif//INWIN_H