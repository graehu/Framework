#include "update.h"
#include <algorithm>


std::map<update::group, std::vector<update::callback>> update::m_updates;
void update::run(float _deltatime)
{
   for(auto update : m_updates)
   {
      for(int i = 0; i < update.second.size(); i++)
      {
	 update.second[i](_deltatime);
      }
   }
}
void update::subscribe(group _group, callback _callback)
{
   auto update = m_updates.find(_group);
   if(update != m_updates.end())
   {
      update->second.push_back(_callback);
   }
}

void update::unsubscribe(group _group, callback _callback)
{
   auto update = m_updates.find(_group);
   if(update != m_updates.end())
   {
      auto cb = std::find(update->second.begin(), update->second.end(), _callback);
      if(cb != update->second.end())
      {
	 update->second.erase(cb);
      }
   }
}

