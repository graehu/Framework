#ifndef WINDOW_H
#define WINDOW_H

class window
{
 public:

  virtual int init(int _width, int _height, const char* _name) = 0;
  virtual int move(int _x, int _y) = 0;
  virtual int resize(int _width, int _height) = 0;
  int getHeight(){return m_height;}
  int getWidth(){return m_width;}
  static window* windowFactory();

 protected:

  int m_width;  //window width
  int m_height; //window height
  const char* m_name; //window name

 private:

};

#endif//WINDOW_H
