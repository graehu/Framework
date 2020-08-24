#include "packet.h"
#include "../../utils/log/log.h"

#include <cstdio>
#include <cstdarg>
#include <fstream>

using namespace net;

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
bool BasePacket::OpenFile(const char* _file_name)
{
   if (m_file == nullptr)
   {
      return false;
   }
   if(!m_file->is_open())
   {
      m_file->open(_file_name, std::ios_base::in);
      m_file->seekg(0, m_file->end);
      if(m_file->fail())
      {
	 return false;
      }
      mf_file_size = m_file->tellg();
      mv_file_name = _file_name;
      return true;
   }
   //todo return true if it's the same file, somehow.
   return false;
}
bool BasePacket::IsFileOpen()
{
   return m_file->is_open();
}
int BasePacket::WriteFile(size_t _start, size_t _end, size_t max_write)
{
   if(m_file != nullptr && m_file->is_open())
   {
      if (_end == 0 || _end > GetFileSize())
      {
	 _end = GetFileSize();
      }
      int bytes_to_write = _end - _start;
      int space_remaining = GetRemainingSpace();
      if (max_write > 0)
      {
	 space_remaining = max_write < space_remaining ? max_write : space_remaining;
      }
      int read_length = space_remaining < bytes_to_write ? space_remaining : bytes_to_write;
      m_file->seekg(_start, m_file->beg);
      m_file->read((char*)(&GetData()[end]), read_length);
      end += read_length;
      return read_length;
   }
   return -1;
}
void BasePacket::CloseFile()
{
   if(m_file != nullptr && m_file->is_open())
   {
      m_file->close();
      mf_file_size = 0;
      mv_file_name = nullptr;
   }
}

void BasePacket::PrintDetails()
{
   log::info("[packet] printing %ld bytes:", end);
   log::info("[packet_start]\n");
   log::no_topic("%.*s", end, GetData());
   log::info("[packet_end]");
}    
  





///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///Code Graveyard

packet::packet()
{
	m_alloc = 0;
	m_packing = false;
	m_headerSize = m_end = 16;
    m_data = new unsigned char[m_headerSize];
    m_sizeAccumulator = 0;
	m_dataEnd = 0;
	//m_typeIttorator = 0;
    //m_currentDefID = 0;
}

packet::~packet()
{ ///this may require more later.
    if(m_data != 0)
	delete m_data;
}


bool packet::setAlloc(unsigned int _alloc)
{
	if(_alloc > m_alloc)
	{
		unsigned char* temp;
		temp = m_data;
		m_data = new unsigned char[_alloc];
		if(temp != 0)
		{
            memcpy(&m_data[0], temp, m_end);
            delete temp;
		}
		m_alloc = _alloc;

		return false;
	}
	return true;
}

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
