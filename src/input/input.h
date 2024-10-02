#ifndef INPUT_H
#define INPUT_H

class input
{
 public:

enum keys
  {	
    e_up = 1,
    e_down,
    e_left,
    e_right,
    e_respawn,
    e_quit,
    e_totalKeys
  };
enum mouseButtons
{
	e_leftClick = 0,
	e_rightCLick,
	e_middleClick, //scrolling has to be done in an event loop with sdl so meh.
	e_totalButtons
};
  virtual ~input() = 0;
  virtual int init(void) = 0; //This functions should read What keys will be assigned to the commands above.
  virtual bool update(void) = 0;
  virtual bool isKeyPressed(keys _key) = 0;
  virtual bool isMouseClicked(mouseButtons _button) = 0;
  virtual void mouseDelta(float& _dx, float& _dy) = 0;

  static input* inputFactory(void);

 protected:

  bool m_keys[e_totalKeys];
  bool m_mouseButtons[e_totalButtons];

 private:

};

#endif/*INPUT_H*/
