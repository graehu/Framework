#ifndef WINDOW_H
#define WINDOW_H

class window
{
 public:

  virtual int init(int _width, int _height, char* _name) = 0;
  virtual int move(int _x, int _y) = 0;
  virtual int resize(int _width, int _height) = 0;
  virtual int getHeight() = 0;
  virtual int getWidth() = 0;
  static window* windowFactory();

 protected:

  int m_width;  //window width
  int m_height; //window height
  char* m_name; //window name

 private:

};

#endif//WINDOW_H
