#include "commandline.h"
#include "log/log.h"
#include "log/log_macros.h"
#include "hasher.h"
#include <bits/c++config.h>
#include <cstdint>
#include <cstring>

namespace commandline
{
   std::map<std::uint32_t, std::unique_ptr<params::param>> params::m_params;
   int arg_count = 0;
   char** arg_variables = nullptr;
   void parse(int argc, char *argv[])
   {
      log::topics::add("commandline");
      log::topics::level("commandline", log::e_debug);
      auto topic = log::scope("commandline");
      log::debug("parsing params:");
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
	       //todo: skip if level < debug?
	       log::debug("\t-%s", current->m_name, hash);
	       for(auto arg : current->m_args)
	       {
		  log::debug("\t\t%s", arg);
	       }
	       params::m_params.emplace(hash, current.release());
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
	 //todo:: skip if level < debug?
	 log::debug("\t-%s", current->m_name, hash);
	 for(auto arg : current->m_args)
	 {
	    log::debug("\t\t%s", arg);
	 }
	 params::m_params.emplace(hash, current.release());
      }
   }
}
