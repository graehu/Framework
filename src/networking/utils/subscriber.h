#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

namespace net
{
enum events
{
	e_newEntityEvent = 0
};

class subscriber
{
public:
	virtual void notify(events) = 0;
};

}
#endif//SUBSCRIBER_H
