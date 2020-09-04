#ifndef PARAMS_H
#define PARAMS_H
#include <cstdint>
#include <memory>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include "hasher.h"

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
   // usage: params::add("path.to.param", {"1", "2", "3"})
   template<typename T>
   static bool add(T&& _path, param_args _args)
   {
      m_mutex.lock();
      bool return_val = false;
      if(!exists(_path))
      {
	 m_hashes.emplace(hash::i32(_path));
	 hash::path in_path = hash::make_path(_path);
	 return_val =  m_params.add(in_path, _args);
      }
      m_mutex.unlock();
      return return_val;
   }
   
   // adds the param at path with args if it doesn't exist already.
   // usage: params::add(hash::make_path(var, len), {"1", "2", "3"})
   static bool add(hash::path& _path,  param_args _args)
   {
      m_mutex.lock();
      bool return_val = false;
      if(!exists(_path))
      {
	 m_hashes.emplace(_path.m_hash);
	 return_val = m_params.add(_path, _args);
      }
      m_mutex.unlock();
      return return_val;
   }
   // adds the callback to paths callback list
   // usage: params::subscribe("log.level", this);
   template<typename T>
   static bool subscribe(T&& _path, callback* _callback)
   {
      m_mutex.lock();
      bool return_val = false;
      hash::path in_path = hash::make_path(_path);
      return_val =  m_params.subscribe(in_path, _callback);
      m_mutex.unlock();
      return return_val;
   }
   // adds the callback to paths callback list
   // usage: params::subscribe(hash::make_path(var, len), this);
   static bool subscribe(hash::path& _path,  callback* _callback)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.subscribe(_path, _callback);
      m_mutex.unlock();
      return return_val;
   }
   // removes the callback from paths callback list
   // usage: params::unsubscribe("log.level", this);
   template<typename T>
   static bool unsubscribe(T&& _path, callback* _callback)
   {
      m_mutex.lock();
      bool return_val = false;
      hash::path in_path = hash::make_path(_path);
      return_val =  m_params.unsubscribe(in_path, _callback);
      m_mutex.unlock();
      return return_val;
   }
   // removes the callback from paths callback list
   // usage: params::unsubscribe(hash::make_path(var, len), this);
   static bool unsubscribe(hash::path& _path,  callback* _callback)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.unsubscribe(_path, _callback);
      m_mutex.unlock();
      return return_val;
   }
   // returns true if the param at path exists
   // usage: params::exists("path.to.param")
   template<typename T>
   static bool exists(T&& _path)
   {
      return m_hashes.find(hash::i32(_path)) != m_hashes.end();
   }
   
   // returns true if the param at path exists
   // usage: params::exists(hash::make_path(var, len))
   static bool exists(hash::path& _path)
   {
      return m_hashes.find(_path.m_hash) != m_hashes.end();
   }
   
   // sets the arguments at the path
   // usage: params::set_args("path.to.param", {"1", "2", "3"})
   template<typename T>
   static bool set_args(T&& _path, param_args _args)
   {
      m_mutex.lock();
      bool return_val = false;
      auto in_path = hash::make_path(_path);
      return_val =  m_params.set_args(in_path, _args);
      m_mutex.unlock();
      return return_val;
   }
   
   // sets the arguments at the path
   // usage: params::set_args(hash::make_path(var, len), {"1", "2", "3"})
   static bool set_args(hash::path& _path, param_args _args)
   {
      m_mutex.lock();
      bool return_val = false;
      return_val = m_params.set_args(_path, _args);
      m_mutex.unlock();
      return return_val;
   }

   // gets a value at path of index.
   // usage: params::get_args("path.to.param", 0);
   template<typename T>
   static const char* get_value(T&& _path, int index)
   {
      m_mutex.lock();
      const char* return_val = nullptr;
      auto hash = hash::make_path(_path);
      return_val =  m_params.get_value(hash, index);
      m_mutex.unlock();
      return return_val;
   }
   
   // gets a value at path of index.
   // usage: params::get_args(hash::make_path(hash::make_path(var, len)), 0);
   static const char* get_value(hash::path& _path, int index)
   {
      m_mutex.lock();
      const char* return_val = nullptr;
      return_val = m_params.get_value(_path, index);
      m_mutex.unlock();
      return return_val;
   }
   
   // gets the arguenents at path.
   // usage: params::get_args("path.to.param");
   template<typename T>
   static param_args get_args(T&& _path)
   {
      m_mutex.lock();
      param_args return_val;
      auto hash = hash::make_path(_path);
      return_val =  m_params.get_args(hash);
      m_mutex.unlock();
      return return_val;
   }
   
   // gets the arguenents at path.
   // usage: params::get_args(hash::make_path(var, len));
   static param_args get_args(hash::path& _path)
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
      bool add(hash::path& _path, param_args _args, int _depth = 0);
      // Adds callback to paths callback list
      bool subscribe(hash::path& _path, callback* _callback, int _depth = 0);
      // Removes callback from paths callbacks list
      bool unsubscribe(hash::path& _path, callback* _callback, int _depth = 0);
      // Gets a value at the path and index
      const char* get_value(hash::path& _path, int index, int _depth = 0);
      // Gets the arguements at the path
      param_args get_args(hash::path& _path, int _depth = 0);
      // Sets the arguments at the path
      bool set_args(hash::path& _path, param_args _args, int _depth = 0);
      //
      void print();
      //
      std::uint32_t m_hash = 0;
      std::string m_name;
      param_args m_args;
      std::set<params::callback*> m_callbacks;
      std::map<std::uint32_t, std::unique_ptr<param>> m_params;
   };
   friend void commandline::parse(int argc, char *argv[]);
   static std::recursive_mutex m_mutex;
   static std::set<std::uint32_t> m_hashes;
   static param m_params;
};

#endif//PARAMS_H
