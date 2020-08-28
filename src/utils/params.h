#ifndef PARAMS_H
#define PARAMS_H
#include <cstdint>
#include <memory>
#include <map>
#include <vector>
#include "hasher.h"

namespace commandline
{
   extern int arg_count;
   extern char** arg_variables;
   void parse(int argc, char *argv[]);
   void parse();
}
using param_args = std::vector<const char*>;
class params
{
public:
   // adds the param at path with args if it doesn't exist already.
   // usage: params::add("path.to.param", {"1", "2", "3"})
   template<typename T>
   static bool add(T&& _path, param_args _args)
   {
      if(!exists(_path))
      {
	 hash::path* in_path = new hash::path(hash::make_path(_path));
	 m_paths.emplace(hash::i32(_path), in_path);
	 return m_params.add(*in_path, _args);
      }
      return false;
   }
   
   // adds the param at path with args if it doesn't exist already.
   // usage: params::add(var, len, {"1", "2", "3"})
   static bool add(hash::path& _path,  param_args _args)
   {
      if(!exists(_path))
      {
	 hash::path* in_path = new hash::path(_path);
	 m_paths.emplace(in_path->m_hash, in_path);
	 return m_params.add(*in_path, _args);
      }
      return false;
   }
   
   // returns true if the param at path exists
   // usage: params::exists("path.to.param")
   template<typename T>
   static bool exists(T&& _path)
   {
      return m_paths.find(hash::i32(_path)) != m_paths.end();
   }
   
   // returns true if the param at path exists
   // usage: params::exists(var)
   static bool exists(hash::path& _path)
   {
      return m_paths.find(_path.m_hash) != m_paths.end();
   }
   
   // sets the arguments at the path
   // usage: params::set_args("path.to.param", {"1", "2", "3"})
   template<typename T>
   static bool set_args(T&& _path, param_args _args)
   {
      auto in_path = hash::make_path(_path);
      return m_params.set_args(in_path, _args);
   }
   
   // sets the arguments at the path
   // usage: params::set_args(var, {"1", "2", "3"})
   static bool set_args(hash::path& _path, param_args _args)
   {
      return m_params.set_args(_path, _args);
   }

   // gets a value at path of index.
   // usage: params::get_args("path.to.param", 0);
   template<typename T>
   static const char* get_value(T&& _path, int index)
   {
      auto hash = hash::make_path(_path);
      return m_params.get_value(hash, index);
   }
   
   // gets a value at path of index.
   // usage: params::get_args(hash::make_path(var, len), 0);
   static const char* get_value(hash::path& _path, int index)
   {
      return m_params.get_value(_path, index);
   }
   
   // gets the arguenents at path.
   // usage: params::get_args("path.to.param");
   template<typename T>
   static param_args get_args(T&& _path)
   {
      auto hash = hash::make_path(_path);
      return m_params.get_args(hash);
   }
   
   // gets the arguenents at path.
   // usage: params::get_args(hash::make_path(var));
   static param_args get_args(hash::path& _path)
   {
      return m_params.get_args(_path);
   }
   //
   static void print();
private:
   
   class param
   {
   public:
      param(){}
      param(const char* _name, param_args _args) : m_name(_name), m_args(_args) {}
      // Adds a param at the path
      bool add(hash::path& _path, param_args _args, int _depth = 0);
      // Gets a value at the path and index
      const char* get_value(hash::path& _path, int index, int _depth = 0);
      // Gets the arguements at the path
      param_args get_args(hash::path& _path, int _depth = 0);
      // Sets the arguments at the path
      bool set_args(hash::path& _path, param_args _args, int _depth = 0);
      //
      std::uint32_t m_hash = 0;
      const char* m_name = nullptr;
      param_args m_args;
      std::map<std::uint32_t, std::unique_ptr<param>> m_params;
   };
   friend void commandline::parse(int argc, char *argv[]);
   static std::map<std::uint32_t, std::unique_ptr<hash::path>> m_paths;
   static param m_params;
};

#endif//PARAMS_H
