#ifndef PACKET_H
#define PACKET_H

#include <cstddef>
#include <cstring>
#include <cstdio>
#include <iosfwd>


#include "../utils/dataUtils.h"


namespace net
{
class packet
{
	public:
	packet();
    ~packet();

    /// Packet data related functions

	bool clearPacket(void){m_end = m_headerSize; m_dataEnd = m_headerSize; return false;}
	bool setAlloc(unsigned int _alloc);
	void setEnd(unsigned int _end){if(_end < m_alloc && _end > m_headerSize)m_end = _end;}
	void setDataEnd(unsigned int _end){if(_end < m_alloc && _end > m_headerSize)m_dataEnd = _end;}

	unsigned int getAlloc(void) {return m_alloc;}
	unsigned int getEnd(void){return m_end;}
	unsigned int getDataEnd(void){return m_dataEnd;}
	unsigned char* getData(void){return m_data;}
	unsigned int getHeaderSize(void){return m_headerSize;}

	template <typename T>
	void iterWrite(T _type)
	{
		dataUtils::instance().writeData(_type, &m_data[m_end]);
		m_end += sizeof(T);
	}
	template <typename T>
	T iterRead(void)
	{
		T temp =  dataUtils::instance().readData<T>(&m_data[m_end]);
		m_end += sizeof(T);
		return temp;
	}

	protected:


	unsigned int m_alloc;  // amount of memory currently allocated to the packet
	unsigned char* m_data; // the actual data
	unsigned int m_end;   // the End offset, how far it is to the end of the current data-set
	unsigned int m_dataEnd; //Were the end of meaningful data is.
	bool m_packing;



	private:

	unsigned int m_sizeAccumulator;
	unsigned int m_headerSize;   /// = 16;
};
  //
  class BasePacket
  {
  public:
     BasePacket();
     ~BasePacket();
    // Clears the packet of data
    virtual void Clear(void) = 0;

    // returns the size of the valid data in the packet.
    virtual size_t GetSize() = 0;

    // returns the size of the capacity of the packet, that can't grow.
    virtual size_t GetCapacity() = 0;

    // returns the amount of space remaining in the packet
    size_t GetRemainingSpace() { return GetCapacity()-end; }

    // Get the packet data used by sockets
    virtual unsigned char* GetData() = 0;

    // Sets the length of the valid data in the packet.
    virtual void SetLength(size_t a_length) = 0;
    virtual void PrintDetails() = 0;

    // Write the type data into the packet and
    // shift the write point forward by type size
    template <typename T>
    void IterWrite(T& _type)
    {
      // printf("writing: %ld\n", sizeof(T));
      memcpy(&GetData()[end], &_type, sizeof(T));
      end += sizeof(T);
      if(GetData()[end-1] == '\0')
	{
	  end--;
	}
    }

    // Write the data into the packet and
    // shift the write point forward by size
    void IterWrite(const char* in, size_t size)
    {
      // printf("writing: %ld\n", sizeof(T));
      memcpy(&GetData()[end], in, size);
      end += size;
      if(GetData()[end-1] == '\0')
	{
	  end--;
	}
    }

    
    // Read the type data out of the data and
    // shift the read point forward by sizeof(_type)
    template <typename T>
    T IterRead(void)
    {
      //todo: make safe?
      T temp = *((T*)&GetData()[end]);
      end += sizeof(T);
      return temp;
    }
    void IterRead(unsigned char& buffer, size_t read_length)
    {
      //todo: make safe?
      memcpy(&buffer, &GetData()[end], read_length);
      end += read_length;
    }
     // open the file and prepare to start reading into the packet
     bool OpenFile(const char* _file_name);
     // returns if file is open
     bool IsFileOpen();
     // Write an std stream at current position
     int WriteFile(size_t start = 0, size_t end = 0, size_t max_write = 0);
     // try close the file.
     void CloseFile();
     // Get file bytes written
     int GetFileSize() { return mf_file_size; }
     // get file name/path
     const char* GetFileName() { return mv_file_name; }


  protected:
    // The end offset, how far it is to the end of valid data
    size_t end = 0;
    int mf_file_size = 0;
     const char* mv_file_name = nullptr;
    std::fstream* m_file;
  };
  
  //allows for zero size header.
  typedef int null_type[0];
  //
  template<size_t T_size, typename T_header = null_type>
  class NewPacket : public BasePacket
  {
  public:
    
    // Clears the packet of data
    void Clear(void){ end = sizeof(T_header); }

    // returns the size of the valid data in the packet.
    size_t GetSize() { return end; }

    // returns the size of the capacity of the packet, that can't grow.
    size_t GetCapacity() { return T_size; }

    // Sets the size of the valid data in the packet.
    void SetLength(size_t a_length)
    {
      end = a_length < T_size ? a_length : T_size;
      data[end-1] = '\0';
    }

    // if valid is true, this prints the valid contents of the packet.
    // else it prints the packet capacity
    void PrintDetails()
    {
      printf("[packet] printing %ld bytes:\n", end);
      printf("[packet_start]\n");
      for(int i = 0; i < end; i++)
	{
	  printf("%c", data[i]);
	}
      printf("\n[packet_end]\n");
    }    
    // Get the packet data used by sockets
    unsigned char* GetData() { return data; }
    
    // Write the header into the data
    void WriteHeader(T_header _header)
    {
      memcpy(&data[0], &_header, sizeof(_header));
    }

    // Read the header from the data
    void ReadHeader(T_header &_header)
    {
      T_header temp;
      memcpy(&temp, &data[0], sizeof(_header));
      _header = temp;
    }


  protected:
    // The data, sent or recieved.
    unsigned char data[T_size];
    


  };
}

#endif//PACKET_H














////Code graveyard

//public:
/// header realated functionality.



	/// stupid template functions can't be declared in one file and defined in another.


  /// header related functionality.
  //void analysePacket(void);
  //unsigned int getSectionType(unsigned int _section);
  //unsigned int getSectionStart(unsigned int _section);
  //unsigned char* getData(void){return m_data;}

  /*bool beginPacketData(unsigned int _packetDefID);
	template <typename T>
	bool safePushData(T _data)
	{
	    if(m_packing)
	    {
	    	char* name = defRegister::instance().getDef(m_currentDefID).getMembTypeName(m_typeIttorator);
          if(typeid(T).name() == name)
          {
              unsigned int size = sizeof(_data);
              if((m_end + size) > m_alloc)
              {
                  unsigned char* temp;
                  temp = m_data;
                  m_data = new unsigned char[(m_end + size)*2];
                  if(temp != 0)
                  {
                      memcpy(&m_data[0], temp, m_end);
                      delete temp;
                  }
                  m_alloc = (m_end + size)*2;
              }
              //printf("i'm pushing, right?\n");
              writeData(_data,m_end);

              m_sizeAccumulator += size;
              m_end += size;
              m_typeIttorator++;
              return false;
          }
          else
          {
              printf("packet: safePushData(), error expected:%s passed:%s\n",
                     (const char*)name,
                     typeid(T).name());
              return true;
          }
		}
		else
		{
		    printf("packet: safePushDate(), error haven't begun packet\n");
          return true;
		}
	};
  bool endPacketData(void);*/

  /*template <typename T>
  void getMembData(unsigned int _section, unsigned int _member, T &_data)
  {
      if(_section < m_sectionInfo.size())
      {
          if(_member < m_sectionInfo[_section].size())
          {
              unsigned int offset = m_sectionInfo[_section][_member].second;
              if(typeid(_data).name() != m_sectionInfo[_section][_member].first)
              	printf("packet: getMemData(), warning passing incorrect type to be filled.\n");
              readDataIntoType(offset, _data);
          }
      }
  }*/

  /*template <typename T>
  void readDataIntoType(unsigned short _offset, T &_data)
  {
      if(typeid(_data) == typeid(short))
      {
          _data = readShort(_offset);
          return;
      }
      if(typeid(_data) == typeid(unsigned short))
      {
          _data = readUShort(_offset);
          return;
      }
      if(typeid(_data) == typeid(int))
      {
          _data = readInteger(_offset);
          return;
      }
      if(typeid(_data) == typeid(unsigned int))
      {
          _data = readUInteger(_offset);
          return;
      }
      if(typeid(_data) == typeid(char))
      {
          _data = readChar(_offset);
          return;
      }
      if(typeid(_data) == typeid(unsigned char))
      {
          _data = readUChar(_offset);
          return;
      }

      printf("failure on type recognition\n");
  }*/

//private:

//TODO turn this into a definition pointer?
//TODO at least make this use packType instead of char*
//std::vector<std::vector<std::pair<char*, unsigned int> > > m_sectionInfo; ///vector<type(name)> and offset(start of section).
//std::vector<unsigned int> m_sectionIDs;
//std::vector<std::vector<std::pair<packType, unsigned int> > > m_sectionInfo;
//unsigned int m_currentDefID;
//unsigned int m_typeIttorator;


