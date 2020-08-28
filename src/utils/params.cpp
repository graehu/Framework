#include "params.h"
#include "log/log.h"
#include "log/log_macros.h"
#include "hasher.h"
#include <bits/c++config.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <functional>

auto params_topic = log::topics::add("params");
std::map<std::uint32_t, std::unique_ptr<hash::path>> params::m_paths;
params::param params::m_params;

namespace commandline
{
   int arg_count = 0;
   char** arg_variables = nullptr;
   void parse(int argc, char *argv[])
   {
      int start = 0, end = 0;
      auto current = std::make_unique<params::param>();
      for(int i = 0; i < argc; i++)
      {
	 if(argv[i][0] == '-')
	 {
	    if(current->m_name == nullptr)
	    {
	       current->m_name = ++argv[i];
	    }
	    else
	    {
	       std::size_t len = std::strlen(current->m_name);
	       if(params::exists(current->m_name, len))
	       {
		  if(!params::set_args(current->m_name, len, current->m_args))
		  {
		     log::debug("wrong args used for %s", current->m_name);
		  }
	       }
	       else
	       {
		  if(!params::add(current->m_name, len, current->m_args))
		  {
		     log::debug("something went wrong adding %s", current->m_name);
		  }
	       }
	       current = std::make_unique<params::param>();
	       current->m_name = ++argv[i];
	    }
	 }
	 else if(current->m_name != nullptr)
	 {
	    current->m_args.push_back(argv[i]);
	 }
      }
      if(current->m_name != nullptr)
      {
	 std::size_t len = std::strlen(current->m_name);
	 if(params::exists(current->m_name, len))
	 {
	    if(!params::set_args(current->m_name, len, current->m_args))
	    {
	       log::debug("wrong args used for %s", current->m_name);
	    }
	 }
	 else
	 {
	    if(!params::add(current->m_name, len, current->m_args))
	    {
	       log::debug("something went wrong adding %s", current->m_name);
	    }
	 }
      }
      params::print();
   }
   void parse()
   {
      parse(arg_count, arg_variables);
   }
}
void params::print()
{
   log::topics::level("params", log::e_debug);
   auto topic = log::scope("params");
   log::debug("----------------");
   log::debug("printing params:");
   for(auto it = m_paths.begin(); it != m_paths.end(); it++)
   {
      std::string param_path;
      for(int i = 0; i < it->second->m_name_count; i++)
      {
	 param_path += it->second->m_names[i];
	 if(i < it->second->m_name_count -1)
	 {
	    param_path += ".";
	 }
      }
      param_args args = params::get_args(*it->second.get());
      for(auto arg : args)
      {
	 param_path += " ";
	 param_path += arg;
      }
      param_path += " : hash ";
      param_path += std::to_string(it->first);
      log::debug("%s", param_path.c_str());
   }
}

bool params::param::add(hash::path& _path, param_args _args, int _depth)
{
   int i = _depth++;
   auto it = m_params.find(_path.m_hashes[i]);
   if(it == m_params.end())
   {
      auto current_param = new param();
      current_param->m_name = _path.m_names[i];
      current_param->m_hash = _path.m_hashes[i];
      m_params.emplace(_path.m_hashes[i], current_param);
      if(_path.m_name_count > i+1)
      {
	 return current_param->add(_path, _args, _depth);
      }
      else
      {
	 current_param->m_args = _args;
	 return true;
      }
   }
   else if(_path.m_name_count > i+1)
   {
      return it->second->add(_path, _args, _depth);
   }
   return false;
}
const char* params::param::get_value(hash::path& _path, int index, int _depth)
{
   int i = _depth++;
   auto it = m_params.find(_path.m_hashes[i]);
   if(it != m_params.end())
   {
      if(_path.m_name_count > i+1)
      {
	 return it->second->get_value(_path, index, _depth);
      }
      else if(it->second->m_args.size() > index)
      {
	 return it->second->m_args[index];
      }
   }
   return nullptr;
}
param_args params::param::get_args(hash::path& _path, int _depth)
{
   int i = _depth++;
   auto it = m_params.find(_path.m_hashes[i]);
   if(it != m_params.end())
   {
      if(_path.m_name_count > i+1)
      {
	 return it->second->get_args(_path, _depth);
      }
      else
      {
	 return it->second->m_args;
      }
   }
   return {};
}
bool params::param::set_args(hash::path& _path, param_args _args, int _depth)
{
   int i = _depth++;
   auto it = m_params.find(_path.m_hashes[i]);
   if(it != m_params.end())
   {
      if(_path.m_name_count > i+1)
      {
	 return it->second->set_args(_path, _args, _depth);
      }
      else if(_args.size() == it->second->m_args.size())
      {
	 it->second->m_args = _args;
	 return true;
      }
   }
   return false;
}
