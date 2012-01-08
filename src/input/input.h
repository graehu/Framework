#ifndef INPUT_H
#define INPUT_H

enum gameKeys
  {
    e_up = 0,
    e_left,
    e_right,
    e_down,
    e_totalKeys
  };

class input
{
 public:

  virtual int init(void) = 0; //This functions should read What keys will be assigned to the commands above.
  virtual bool update(void) = 0;
  virtual bool isKeyPressed(gameKeys _key) = 0;
  virtual int mouse(void) = 0; //hmmmmmmmmmmmmmmm

  static input* inputFactory(void);

 protected:

  bool m_keys[e_totalKeys];

 private:

};

#endif/*INPUT_H*/
