

#include "../input.h"


class inGlfw : public input
{
public:
   int init(void);
   bool isKeyPressed(keys _key);
   bool isMouseClicked(mouseButtons _button);
   bool update(void);
   bool setMousePosition(float _x, float _y);
   bool centerMousePosition();
   void mouseDelta(float& _dx, float& _dy);
};
