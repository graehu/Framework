#ifndef INETWORKED_H
#define INETWORKED_H

class packet;

class iNetworked
{
public:

	virtual void writePacket(packet* _packet) = 0;
	virtual void readPacket(packet* _packet) = 0;

protected:
private:

};

#endif//INETWORKED_H