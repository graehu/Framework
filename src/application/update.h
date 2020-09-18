#ifndef UPDATE_H
#define UPDATE_H

#include <map>
#include <vector>
#include <functional>

//list updates here.
class update
{
  public:
   typedef std::function<void(float _deltatime)> callback;
   enum group
   {
      e_early,
      e_normal,
      e_late
   };
   static void run(float _deltatime);
   static void subscribe(group _group, callback _callback);
   static void unsubscribe(group _group, callback _callback);
   
  private:
   static std::map<group, std::vector<callback>> m_updates;
};

#endif//UPDATE_H
