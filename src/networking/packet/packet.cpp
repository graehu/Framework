#include "packet.h"

#include <cstdio>
#include <cstdarg>
#include <fstream>

using namespace net;

#define show_val(variable) printf(#variable": %d\n", variable);

BasePacket::BasePacket()
{
   m_file = new std::fstream;
}

BasePacket::~BasePacket()
{ ///this may require more later.
   if(m_file != nullptr)
   {
      delete m_file;
      m_file = nullptr;
   }
}
BasePacket::file_write_status BasePacket::OpenFile(const char* _file_name)
{
   if (m_file == nullptr)
   {
      return e_failed;
   }
   if(!m_file->is_open())
   {
      m_file->open(_file_name, std::ios_base::in);
      mf_file_bytes_written = 0;
      m_file->seekg(0, m_file->end);
      mf_file_size = m_file->tellg();
      if(m_file->fail())
      {
	 return e_failed;
      }
      return e_in_progress;
   }
   else if(mf_file_bytes_written == mf_file_size)
   {
      return e_complete;
   }
   else
   {
      return e_in_progress;      
   }
}
BasePacket::file_write_status BasePacket::WriteFile(size_t max_write)
{
   if(m_file != nullptr && m_file->is_open())
   {
      if(mf_file_bytes_written == mf_file_size)
      {
	 return e_complete;
      }
      int file_remaining = GetFileBytesToWrite();
      int space_remaining = GetRemainingSpace();
      if (max_write > 0)
      {
	 space_remaining = max_write < space_remaining ? max_write : space_remaining;
      }
      int read_length = space_remaining < file_remaining ? space_remaining : file_remaining;
      m_file->seekg(mf_file_bytes_written, m_file->beg);
      m_file->read((char*)(&GetData()[end]), read_length);
      show_val(read_length);
      end += read_length;
      mf_file_bytes_written += read_length;
      if (read_length == file_remaining)
      {
	 return e_complete;
      }
      else
      {
	 return e_in_progress;
      }
   }
   return e_failed;
}
void BasePacket::CloseFile()
{
   if(m_file != nullptr && m_file->is_open())
   {
      m_file->close();
      mf_file_bytes_written = 0;
      mf_file_size = 0;
   }
}

  





///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///Code Graveyard













/*
bool packet::beginPacketData(unsigned int _packetDefID)
{
	unsigned int numOfDefs = defRegister::instance().getSize()-1;
    if(_packetDefID < numOfDefs)
    {
        assert(!m_packing);
        if(!m_packing)
        {
            m_currentDefID = _packetDefID;
            m_packing = true;
            safePushData((unsigned short)m_currentDefID);
            return false;
        }
        else return true;
    }
    else
    {
        printf("packet: beginPacketData(), error aPacketDefID too large\n");
        return true;
    }
}

bool packet::endPacketData()
{
    if(m_packing)
    {
    	unsigned int size = defRegister::instance().getDef(m_currentDefID).getSize();
        if(m_sizeAccumulator != size)
        {
            printf("packet: endPacketData(), warning packet size inadequate.\n");
            //assert(false);
        }
        m_sizeAccumulator = 0;
        m_typeIttorator = 0;
        m_currentDefID = 0;
        m_packing = false;
        return false;
    }
    else return true;
}


void packet::analysePacket(void)
{
    unsigned short offset = m_headerSize;
    unsigned short secType = readUShort(offset);
    unsigned int size = defRegister::instance().getSize()-1;
    bool data = true;
    unsigned int i = 0;
    unsigned int ii = 0;

    m_sectionIDs.push_back(secType);

    if(secType < size)
    {
        while(data)
        {
        	char* name = defRegister::instance().getDef(secType).getMembTypeName(ii);

            while(name != 0)
            {
            	packType type;
                if(i >= m_sectionInfo.size())
                    m_sectionInfo.push_back(std::vector<std::pair<char*, unsigned int> >());

                if(ii >= m_sectionInfo[i].size())
                	m_sectionInfo[i].push_back(std::pair<char*,unsigned short>(name,offset));
                else
                {
                    m_sectionInfo[i][ii].first = name;
                    m_sectionInfo[i][ii].second = offset;

                }
                offset += defRegister::instance().getTypeSizeByName(name);
                ii++;
                name = defRegister::instance().getDef(secType).getMembTypeName(ii);
            }
            secType = readUShort(offset);
            m_sectionIDs.push_back(secType);
            ii = 0;
            i++;
            if(secType == 0)
                data = false;
        }
    }
    else
    {
    	//printf("last sectype: %i\n");
    	assert(false);
    	return;
    }
}

unsigned int packet::getSectionType(unsigned int _section)
{
    if(_section < m_sectionInfo.size() && _section >= 0)
    {
        if(!m_sectionInfo[_section].empty())
        return readUShort(m_sectionInfo[_section][0].second);

        //printf("packet: getSectionType(), error section = %i, no such section\n", aSection);
        return 0;
    }
    else
    {
        //printf("packet: getSectionType(), error section = %i, no such section\n", aSection);
        return 0;
    }
}

unsigned int packet::getSectionStart(unsigned int _section)
{
    if(_section < m_sectionInfo.size())
    {
        return m_sectionInfo[_section][0].second;
    }
    return 0;
}*/
