#include "params.h"
#include "log/log.h"
#include "log/log_macros.h"
#include "hasher.h"
#include <bits/c++config.h>
#include <cstdint>
#include <cstring>
std::map<std::uint32_t, std::unique_ptr<params::param>> params::m_params;
auto params_topic = log::topics::add("params");

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
	       std::uint32_t hash = hash::i32(current->m_name, len);
	       auto it = params::m_params.find(hash);
	       if(it == params::m_params.end())
	       {
		  params::m_params.emplace(hash, current.release());
	       }
	       else
	       {
		  if(current->m_args.size() == it->second->m_args.size())
		  {
		     it->second->m_args = current->m_args;
		  }
		  else
		  {
		     log::debug("wrong args used for %s", current->m_name);
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
	 std::uint32_t hash = hash::i32(current->m_name, len);
	 auto it = params::m_params.find(hash);
	 if(it == params::m_params.end())
	 {
	    params::m_params.emplace(hash, current.release());
	 }
	 else
	 {
	    if(current->m_args.size() == it->second->m_args.size())
	    {
	       it->second->m_args = current->m_args;
	    }
	    else
	    {
	       log::debug("wrong args used for %s", current->m_name);
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
   for(auto it = m_params.begin(); it != m_params.end(); it++)
   {
      log::debug("-%s", it->second->m_name);
      for(auto arg : it->second->m_args)
      {
	 log::debug("\t%s,", arg);
      }  
   }
   log::debug("----------------");
}
