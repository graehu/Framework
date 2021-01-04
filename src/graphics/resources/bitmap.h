#ifndef BITMAP_H
#define BITMAP_H

/*
#doc: loads a bitmap file into memory, only supports 24byte bitmaps
#todo: support 256bit bitmap
#todo: add logs
#todo: make an image class to wrap this
*/

class bitmap
{
public:
   bitmap(const char* _filename);
   bitmap(int _width, int _height, signed char* _data, int _data_size);
   ~bitmap();
   bool save(const char* _filename);
   signed char* get_data(void) { return m_data; }
   int get_width(void) { return m_width; }
   int get_height(void) { return m_height; }
   signed char get_red_value(unsigned int _u, unsigned int _v);

protected:
   //writen with opengl in mind
   signed char* m_data;
   int m_data_size;
   unsigned int m_width;
   unsigned int m_height;
   bool m_loaded;
   const char* m_filename;
	
private:
};




#endif//BITMAP_H
