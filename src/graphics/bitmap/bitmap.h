#ifndef BITMAP_H
#define BITMAP_H

class bitmap
{
public:
	bitmap(const char* _filename);
	~bitmap();

	signed char* getBitmapData(void){return m_data;}
	int getWidth(void){return m_width;}
	int getHeight(void){return m_height;}
	signed char getRedVal(unsigned int _u, unsigned int _v);

protected:
	//writen with opengl in mind
	signed char* m_data;
	int m_width;
	int m_height;
	
private:
};




#endif//BITMAP_H