#pragma once

class window
{
 public:

  virtual int init(int _width, int _height, const char* _name) = 0;
  virtual int move(int _x, int _y) = 0;
  virtual int resize(int _width, int _height) = 0;
   virtual int shutdown() = 0;
  int get_height(){ return m_height; }
  int get_width(){ return m_width; }
  static window* windowFactory();

 protected:

  int m_width;  //window width
  int m_height; //window height
  const char* m_name; //window name

 private:

};
