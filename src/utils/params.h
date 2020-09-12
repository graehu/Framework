#ifndef PARAMS_H
#define PARAMS_H
#include <cstdint>
#include <memory>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include "hasher.h"

/*
Params are global variables, which can be set
via the commandline or programatically.

Ideally they should be efficent to lookup and set.
Additions are less time dependent.
*/

// todo: remove top level params mutex and add per param mutexes.
// todo: add "get_param" returning const param object.
// todo: auto migrate subscriptions from parents to params that don't exist yet.

namespace commandline
{
   extern int arg_count;
   extern char** arg_variables;
   void parse(int argc, char *argv[]);
   void parse(char* _string);
   void parse();
}
using param_args = std::vector<std::string>;
class params
{
public:
   class callback
   {
   public:
      // called from add and set, return false if you want to unsub from callback.
      virtual bool param_cb(const char* _param_name, param_args _args) = 0;
      const char* m_cb_name = "m_cb_name";
   };
   
   // adds the param at path with args if it doesn't exist already.
   // usage: params::add(hash::path(var, len), {"1", "2", "3"})
   static bool add(const hash::path& _path,  param_args _args)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.add(_path, _args);
      m_mutex.unlock();
      return return_val;
   }

   // adds the callback to paths callback list
   // usage: params::subscribe(hash::path(var, len), this);
   static bool subscribe(const hash::path& _path,  callback* _callback)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.subscribe(_path, _callback);
      m_mutex.unlock();
      return return_val;
   }

   // removes the callback from paths callback list
   // usage: params::unsubscribe(hash::path(var, len), this);
   static bool unsubscribe(const hash::path& _path,  callback* _callback)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.unsubscribe(_path, _callback);
      m_mutex.unlock();
      return return_val;
   }
   
   // sets the arguments at the path
   // usage: params::set_args(hash::path(var, len), {"1", "2", "3"})
   static bool set_args(const hash::path& _path, param_args _args)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.set_args(_path, _args);
      m_mutex.unlock();
      return return_val;
   }

   
   // gets a value at path of index.
   // usage: params::get_args(hash::path(hash::path(var, len)), 0);
   static const char* get_value(const hash::path& _path, int index)
   {
      m_mutex.lock();
      const char* return_val = nullptr;
      return_val = m_params.get_value(_path, index);
      m_mutex.unlock();
      return return_val;
   }
      
   // gets the arguenents at path.
   // usage: params::get_args(hash::path(var, len));
   static param_args get_args(const hash::path& _path)
   {
      m_mutex.lock();
      param_args return_val;
      return_val = m_params.get_args(_path);
      m_mutex.unlock();
      return return_val;
   }
   //
   static void print();
   
private:
   class param
   {
   public:
      param(){}
      // Adds a param at the path
      bool add(const hash::path& _path, param_args _args, std::uint32_t _depth = 0);
      // Adds callback to paths callback list
      bool subscribe(const hash::path& _path, callback* _callback, std::uint32_t _depth = 0);
      // Removes callback from paths callbacks list
      bool unsubscribe(const hash::path& _path, callback* _callback, std::uint32_t _depth = 0);
      // Gets a value at the path and index
      const char* get_value(const hash::path& _path, std::uint32_t index, std::uint32_t _depth = 0);
      // Gets the arguements at the path
      param_args get_args(const hash::path& _path, std::uint32_t _depth = 0);
      // Sets the arguments at the path
      bool set_args(const hash::path& _path, param_args _args, std::uint32_t _depth = 0);
      //
      void print(const char* _parent);
      //
      std::uint32_t m_hash = 0;
      std::string m_name;
      param_args m_args;
      std::set<params::callback*> m_callbacks;
      std::map<std::uint32_t, std::unique_ptr<param>> m_params;
   };
   friend void commandline::parse(int argc, char *argv[]);
   static std::recursive_mutex m_mutex;
   static param m_params;
};

#endif//PARAMS_H
