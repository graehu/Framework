#include "connection.h"
#include "../utils/dataUtils.h"

using namespace net;
using namespace std;

char message[32];


connection::connection(unsigned short _protocolID, float _timeout, unsigned int _maxSequence)
{
  InitializeSockets();
  m_protocolID = _protocolID;
  m_timeout = _timeout;
  m_maxSequence = _maxSequence;
  m_running = false;
  m_mailList.clear();
  m_sendAccumulator = 0;

  m_bytesRead = 0;
}

connection::~connection()
{
  if(m_running)
    stop();
}

bool connection::start(int _port)
{
  assert(!m_running);
  m_port = _port;
  printf("start connection on port %d\n", _port);
  //sprintf(message, "start connection on port %d\n", _port);
  //OutputDebugString(message);
  if (!m_socket.openSock(_port))
    return false;
  m_running = true;
  return true;
}

void connection::stop()
{
  assert(m_running);
  printf("stop connection\n");
  m_mailList.empty();
  m_socket.closeSock();
  m_running = false;
}

void connection::connect(const address & _address)
{

  printf(message, "client trying: %d.%d.%d.%d:%d\n",
	 _address.getA(),
	 _address.getB(),
	 _address.getC(),
	 _address.getD(),
	 _address.getPort()
	 );

  if(!m_keyPool.empty())   /// If the key pool isn't empty
    {
      /// access mailList based on the key pools returned value
      /// and set mailList element up with the new connections
      /// data
      m_mailList[m_keyPool.back()].first->m_state = e_connecting;
      m_mailList[m_keyPool.back()].first->m_address = _address;
      m_mailList[m_keyPool.back()].first->m_timeoutAccumulator = 0.0f;
      m_mailList[m_keyPool.back()].first->m_stats.reset();
      m_mailList[m_keyPool.back()].second = 0;


      /// add the key onto newConKeys and pop it off of the keyPool
      /// as it is now in use.

      m_newConnKeys.push_back(m_keyPool.back()); ///Receive key
      m_keyPool.pop_back();

      return;
    }

  /// If The key pool Is empty
  /// allocate a new sender
  /// initialise it as a new connection
  /// push it back onto mailList
  /// then push it's array position onto newConKeys

  sender* n_mailer = new sender(m_maxSequence);
  n_mailer->m_address = _address;
  n_mailer->m_state = e_connecting;
  n_mailer->m_timeoutAccumulator = 0.0f;
  unsigned short sendKey = 0;
  m_mailList.push_back(std::pair<sender*, unsigned short>(n_mailer, sendKey));
  m_newConnKeys.push_back(m_mailList.size()-1); /// Receive key
}


void connection::update(float _deltaTime)
{
  assert(m_running);

  bool accumRestart = false;

  if(m_mailList.size() == 0)
    return;
  if(m_sendAccumulator > 1.0f / 30.0f)  /// send if sendAccumulator is greater than 1/30th,
    m_sendAccumulator = 0;

  m_sendAccumulator += _deltaTime; /// advance the sendAccumulator by _deltaTime, important for sending connection packets


  for(unsigned int i = 0; i < m_mailList.size(); i++)  /// loop through all connections
    {
      if(m_mailList[i].first->m_state == e_disconnected)
	continue;

      m_mailList[i].first->m_timeoutAccumulator += _deltaTime;


      /// ///////////////////////////////////////////////////////////////////////////////////////////
      /// ///////////////////////////////////////////////////////////////////////////////////////////
      /// ///////////////////////////////////////////////////////////////////////////////////////////

      /// SECTION OF INTEREST
      /// init packet sending

      if(m_mailList[i].first->m_state == e_connecting) /// send a key for them
        {
	  if(m_sendAccumulator > 1.0f / 30.0f)  ///send if sendAccumulator is greater than 1/30th,
            {
	      ///so roughly 30 a second if send accumulator is a fraction of a second

	      /// i is this elements current position in the array, so if this was the first element
	      /// i would be zero.
		  unsigned char initData[6];
		  dataUtils::instance().writeData(m_protocolID*2, &initData[0]);
		  dataUtils::instance().writeData(m_mailList[i].second, &initData[2]);
		  dataUtils::instance().writeData((unsigned short)i, &initData[4]);
	      m_socket.send(m_mailList[i].first->m_address, initData, 6);

            }
        }

      /// ///////////////////////////////////////////////////////////////////////////////////////////
      /// ///////////////////////////////////////////////////////////////////////////////////////////
      /// ///////////////////////////////////////////////////////////////////////////////////////////


      if (m_mailList[i].first->m_timeoutAccumulator > m_timeout)
        {
	  if (m_mailList[i].first->m_state == e_connecting)
            {
	      printf
                (
		 "Timed-out connecting to: %d.%d.%d.%d:%d\n",
		 m_mailList[i].first->m_address.getA(),
		 m_mailList[i].first->m_address.getB(),
		 m_mailList[i].first->m_address.getC(),
		 m_mailList[i].first->m_address.getD(),
		 m_mailList[i].first->m_address.getPort()
		 );

	      m_mailList[i].first->m_state = e_disconnected;
	      m_mailList[i].first->m_flow.reset();
	      m_mailList[i].second = 0;


	      for(unsigned int ii = 0; ii < m_newConnKeys.size(); ii++)
                {
		  if(m_mailList[i].first->m_address == m_mailList[m_newConnKeys[ii]].first->m_address) ///Removing a connection
                    {
		      if(m_newConnKeys[ii] != m_newConnKeys.back())
                        {
			  /// Switch the current connection out for
			  /// the connection at the back new connection
			  unsigned short temp;
			  temp = m_newConnKeys[ii];
			  m_newConnKeys[ii] = m_newConnKeys.back();
			  m_newConnKeys.back() = temp;
                        }
		      m_newConnKeys.pop_back();
                    }
                }

	      /// push the connection onto the key pool because it can be reused
	      m_keyPool.push_back(i);

	      continue;
            }
	  else if ( m_mailList[i].first->m_state  == e_connected )
            {
	      printf
                (
		 "Connection timed-out: %d.%d.%d.%d:%d\n",
		 m_mailList[i].first->m_address.getA(),
		 m_mailList[i].first->m_address.getB(),
		 m_mailList[i].first->m_address.getC(),
		 m_mailList[i].first->m_address.getD(),
		 m_mailList[i].first->m_address.getPort()
		 );
	      m_mailList[i].first->m_state = e_disconnected;
	      m_mailList[i].first->m_flow.reset();
	      m_mailList[i].second = 0;
	      m_keyPool.push_back(i);
	      continue;
            }
        }
      m_mailList[i].first->m_stats.update(_deltaTime);
    }

}

bool connection::sendPacket(packet* _packet, unsigned short _key, float _deltaTime)
{
  assert(m_running);

  if (m_mailList.size() == 0)
    return false;

  if (m_mailList.size() <= _key)
    return false;

  if (m_mailList[_key].first->m_address.getAddress() == 0)
    return false;

  if( m_mailList[_key].first->m_state != e_connected)
    return false;


  m_mailList[_key].first->m_flow.update(_deltaTime, m_mailList[_key].first->m_stats.getRoundTripTime() * 1000.0f);
  m_mailList[_key].first->m_sendAccumulator += _deltaTime;

  float sendRate = m_mailList[_key].first->m_flow.getSendRate();
  float sendAccumulator = m_mailList[_key].first->m_sendAccumulator;
  bool error = 0;

  if(sendAccumulator > (1.0f/sendRate))
    {

      m_mailList[_key].first->m_sendAccumulator = 0;
	  unsigned char* data = _packet->getData();
	  dataUtils::instance().writeData(m_protocolID, &data[0]); 
      dataUtils::instance().writeData(m_mailList[_key].second, &data[2]); 
      dataUtils::instance().writeData(m_mailList[_key].first->m_stats.getLocalSequence(), &data[4]); 
      dataUtils::instance().writeData(m_mailList[_key].first->m_stats.getRemoteSequence(), &data[8]); 
	  dataUtils::instance().writeData(m_mailList[_key].first->m_stats.generateAckBits(), &data[12]);
	  m_mailList[_key].first->m_stats.packetSent(_packet->getEnd());

      error = m_socket.send(m_mailList[_key].first->m_address, data, _packet->getEnd());
    }

  return error;
}


packet* connection::receivePacket(unsigned int _size)
{
  ////////////////////////////////////////////////////////////////
  /// NOTE                                                     ///
  /// nothing is done with the data that is Received as of yet.///
  ////////////////////////////////////////////////////////////////

  assert(m_running);
  address n_sender;

  packet inPacket;

  inPacket.clearPacket();
  inPacket.setAlloc(_size);
  unsigned char* packetData = inPacket.getData();

  // push this directly into inPacket.
  int l_bytesRead = m_socket.receive(n_sender, packetData, _size);

  if (l_bytesRead == 0)
    return 0;

  inPacket.setEnd(inPacket.getHeaderSize());

  unsigned short security = 0;

  security = dataUtils::instance().readUShort(&packetData[0]);//protocal id check.
  if((security != m_protocolID) && (security != (unsigned short)(m_protocolID*2)))
    return 0;

  bool initPacket = false;
  if(security == (unsigned short)(m_protocolID*2))
    initPacket = true;

  if(initPacket)
    {
	  security = dataUtils::instance().readUShort(&packetData[2]);
      if(security < m_mailList.size())
	{
	  if(m_mailList[security].first->m_address == n_sender)
	    {
	      if(m_mailList[security].first->m_state == e_connected)
		{
		  /// the person sending this init packet is already connected
		  /// so don't perform init operations on this address.
		  return 0;
		}
	    }
	}

      ///checks if the sender is in new connections or not

      for(unsigned int i = 0; i < m_newConnKeys.size(); i++)
        {

	  if(m_mailList[m_newConnKeys[i]].first->m_address == n_sender)
            {
	      /// Look through all the new connections until you find
	      /// the one that belongs to this sender
	      /// set his sent key to what it's telling you
	      /// then check if it is sending the Receive key correctly
		  security = dataUtils::instance().readUShort(&packetData[4]);

	      m_mailList[m_newConnKeys[i]].second = security;
	      security = dataUtils::instance().readUShort(&packetData[2]);


	      if(security == m_newConnKeys[i])
                {
		  ///printf("client connected1\n");

		  printf
                    (
		     "Port: %i Connected to: %d.%d.%d.%d:%d\n",
		     m_port,
		     n_sender.getA(),
		     n_sender.getB(),
		     n_sender.getC(),
		     n_sender.getD(),
		     n_sender.getPort()
		     );

		  /// judging by my output this could be a bad place for these lines.
		  m_mailList[security].first->m_state = e_connected;

		  if(m_newConnKeys[i] != m_newConnKeys.back())
                    {
		      /// Switch the current connection out for
		      /// the connection at the back new connection
		      unsigned short temp;
		      temp = m_newConnKeys[i];
		      m_newConnKeys[i] = m_newConnKeys.back();
		      m_newConnKeys.back() = temp;
                    }
		  m_newConnKeys.pop_back();
                }
	      return 0;
            }
        }

      ///if it is a new connection then give it a key if there is one spare in the key pool
      if(!m_keyPool.empty())
        {
      security = dataUtils::instance().readUShort(&packetData[4]);

	  m_mailList[m_keyPool.back()].first->m_state = e_connecting;
	  m_mailList[m_keyPool.back()].first->m_address = n_sender;
	  m_mailList[m_keyPool.back()].first->m_timeoutAccumulator = 0;
	  m_mailList[m_keyPool.back()].first->m_stats.reset();
	  m_mailList[m_keyPool.back()].second = security;

	  m_newConnKeys.push_back(m_keyPool.back());
	  m_keyPool.pop_back();

	  return 0;
        }
      /// printf("client connecting for first time \n");

      security = dataUtils::instance().readUShort(&packetData[4]);


      sender* n_mailer = new sender(m_maxSequence);
      n_mailer->m_address = n_sender;
      n_mailer->m_state = e_connecting;
      n_mailer->m_timeoutAccumulator = 0.0f;
      n_mailer->m_stats.reset();

      m_mailList.push_back(std::pair<sender*, unsigned short>(n_mailer, security));
      m_newConnKeys.push_back(m_mailList.size()-1);
    }
  else
    {
	  security = dataUtils::instance().readUShort(&packetData[2]);


      if(security > m_mailList.size())
	return 0;

      if(m_mailList[security].first->m_state == e_connecting)
        {
	  for(unsigned int i = 0; i < m_newConnKeys.size(); i++)
            {
	      if(m_mailList[m_newConnKeys[i]].first->m_address == n_sender)
                {
		  /// Look through all the new connections until you find
		  /// the one that belongs to this sender
		  /// then check if it is sending the Receive key correctly

		  //printf("send + 1: %d \n", (int)test);
		  //printf("Receive + 1: %d \n", (int)security);

		  if(security == m_newConnKeys[i])
                    {

		      printf
                        (
			 "Port: %i Connected to: %d.%d.%d.%d:%d\n",
			 m_port,
			 n_sender.getA(),
			 n_sender.getB(),
			 n_sender.getC(),
			 n_sender.getD(),
			 n_sender.getPort()
			 );

		      m_mailList[security].first->m_state = e_connected;
		      ///by this point i must've sent the right key to them

		      if(m_newConnKeys[i] != m_newConnKeys.back())
                        {
			  /// Switch the current connection out for
			  /// the connection at the back new connection
			  unsigned short temp;
			  temp = m_newConnKeys[i];
			  m_newConnKeys[i] = m_newConnKeys.back();
			  m_newConnKeys.back() = temp;
                        }
		      m_newConnKeys.pop_back();
                    }
                }
            }
        }
      
      unsigned int packet_sequence = dataUtils::instance().readUInteger(&packetData[4]);
      unsigned int packet_ack = dataUtils::instance().readUInteger(&packetData[8]);
      unsigned int packet_ack_bits = dataUtils::instance().readUInteger(&packetData[12]);


      m_mailList[security].first->m_stats.packetReceived(packet_sequence, l_bytesRead - 16); ///WHY DOES THIS SAY 14?!.... it doesn't anymore...
      m_mailList[security].first->m_stats.processAck(packet_ack, packet_ack_bits);
      m_mailList[security].first->m_timeoutAccumulator = 0;

	  inPacket.setDataEnd(l_bytesRead);
      return &inPacket;
    }

  return 0;
}
