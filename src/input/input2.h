#pragma once

namespace input2
{
   enum keys
   {	
      e_up = 1,
      e_down,
      e_left,
      e_right,
      e_respawn,
      e_shademode,
      e_nextmodel,
      e_quit,
      e_shift,
      e_totalKeys
   };
   enum mouseButtons
   {
      e_leftClick = 0,
      e_rightCLick,
      e_middleClick, //scrolling has to be done in an event loop with sdl so meh.
      e_totalButtons
   };
   int init(void); //This functions should read What keys will be assigned to the commands above.
   bool update(void);
   bool isKeyPressed(keys _key);
   bool isMouseClicked(mouseButtons _button);
   bool setMousePosition(float _x, float _y);
   bool centerMousePosition();
   void mouseDelta(float& _dx, float& _dy);
}