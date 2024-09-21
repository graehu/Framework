#include "params.h"
#include "log/log.h"
#include "hasher.h"
#include <cstdint>
#include <cstring>
#include <string>

namespace fw
{
   params::param params::m_params;
   std::recursive_mutex params::m_mutex;

   namespace commandline
   {
      int arg_count = 0;
      char** arg_variables = nullptr;
      void parse(int argc, char *argv[])
      {
	 log::topics::add("params");
	 auto topic = log::scope("params");
	 log::debug("parsing: \n");
	 for(int i = 0; i < argc; i++)
	 {
	    log::debug_inline("{}", argv[i]);
	 }
	 log::debug_inline("\n\n");
	 auto current = std::make_unique<params::param>();
	 for(int i = 0; i < argc; i++)
	 {
	    if(argv[i][0] == '-')
	    {
	       if(current->m_name.empty())
	       {
		  current->m_name = ++argv[i];
	       }
	       else
	       {
		  std::size_t len = current->m_name.length();
		  auto param_path = hash::path(current->m_name.c_str(), len);
		  if(!params::add(param_path, current->m_args))
		  {
		     if(!params::set_args(param_path, current->m_args))
		     {
			// #todo: replace string.c_str() within log::debugs
			log::debug("couldn't add or set  {}", current->m_name.c_str());
		     }
		  }
		  current = std::make_unique<params::param>();
		  current->m_name = ++argv[i];
	       }
	    }
	    else if(!current->m_name.empty())
	    {
	       current->m_args.push_back(argv[i]);
	    }
	 }
	 if(!current->m_name.empty())
	 {
	    std::size_t len = current->m_name.length();
	    auto param_path = hash::path(current->m_name.c_str(), len);
	    if(!params::add(param_path, current->m_args))
	    {
	       if(!params::set_args(param_path, current->m_args))
	       {
		  log::debug("couldn't add or set  {}", current->m_name.c_str());
	       }
	    }
	 }
	 params::print();
      }
      void parse()
      {
#if defined(DEFAULT_PARAMS)
	 char default_params[] = { QUOTE(DEFAULT_PARAMS) };
	 parse(default_params);
#endif
	 parse(arg_count, arg_variables);
      }
      void parse(char* _string)
      {
	 const std::uint32_t kMaxArgs = 64;
	 std::uint32_t argc = 0;
	 char *argv[kMaxArgs];
	 char *p2 = std::strtok(_string, " ");
	 while (p2 && argc < kMaxArgs-1)
	 {
	    argv[argc++] = p2;
	    p2 = strtok(0, " ");
	 }
	 argv[argc] = 0;
	 parse(argc, argv);
      }
   }
   void params::print()
   {
      auto topic = log::scope("params");
      log::debug("----------------");
      log::debug("printing params:\n");
      for(auto it = m_params.m_params.begin(); it != m_params.m_params.end(); it++)
      {
	 it->second->print(nullptr);
      }
      log::debug_inline("\n");
   }
   void params::param::print(const char* _parent)
   {
      if(!m_params.empty())
      {
	 for(auto it = m_params.begin(); it != m_params.end(); it++)
	 {
	    if(_parent != nullptr)
	    {
	       log::debug_inline("{}.", _parent);
	    }
	    it->second->print(m_name.c_str());
	 }
      }
      else
      {
	 if(_parent != nullptr)
	 {
	    log::debug_inline("{}.", _parent);
	 }
	 log::debug_inline("{} ", m_name.c_str());
	 for(auto arg : m_args)
	 {
	    log::debug_inline("{} ", arg.c_str());
	 }
	 log::debug_inline("\n");
      }
   }

   bool params::param::add(const hash::path& _path, const param_args& _args, std::uint32_t _depth)
   {
      bool return_val = false;
      std::uint32_t i = _depth++;
      auto it = m_params.find(_path.m_hashes[i]);
      if(it == m_params.end())
      {
	 param* current_param = new param();
	 current_param->m_name = _path.m_names[i];
	 current_param->m_name.resize(_path.m_str_lens[i]);
	 current_param->m_hash = _path.m_hashes[i];
      
	 m_params.emplace(_path.m_hashes[i], std::move(current_param));
	 if(_path.m_name_count > i+1)
	 {
	    return_val = current_param->add(_path, _args, _depth);
	 }
	 else
	 {
	    current_param->m_args = _args;
	    return_val = true;
	 }
      }
      else if(_path.m_name_count > i+1)
      {
	 return_val = it->second->add(_path, _args, _depth);
      }
      if(return_val)
      {
	 std::set<callback*> removals;
	 for(auto cb : m_callbacks)
	 {
	    if(cb != nullptr)
	    {
	       if(!cb->param_cb(m_name.c_str(), m_args))
	       {
		  removals.insert(cb);
	       }
	    }
	 }
	 for(auto cb : removals)
	 {
	    log::debug("unsubscribing {} from {} param", cb->m_cb_name, _path.m_path);
	    m_callbacks.erase(cb);
	 }
      }
      return return_val;
   }
   bool params::param::subscribe(const hash::path& _path, params::callback* _callback, std::uint32_t _depth)
   {
      bool return_val = false;
      std::uint32_t i = _depth++;
      auto it = m_params.find(_path.m_hashes[i]);
      if(it != m_params.end())
      {
	 if(_path.m_name_count > i+1)
	 {
	    return_val = it->second->subscribe(_path, _callback, _depth);
	 }
	 else
	 {
	    return_val = it->second->m_callbacks.insert(_callback).second;
	    if(return_val)
	    {
	       log::debug("subscribed {} to: {} param", _callback->m_cb_name, _path.m_path);
	    }
	 }
      }
      return return_val;
   }
   bool params::param::unsubscribe(const hash::path& _path, params::callback* _callback, std::uint32_t _depth)
   {
      bool return_val = false;
      std::uint32_t i = _depth++;
      auto it = m_params.find(_path.m_hashes[i]);
      if(it != m_params.end())
      {
	 if(_path.m_name_count > i+1)
	 {
	    return_val = it->second->unsubscribe(_path, _callback, _depth);
	 }
	 else
	 {
	    auto it2 = it->second->m_callbacks.find(_callback);
	    if(it2 != it->second->m_callbacks.end())
	    {
	       it->second->m_callbacks.erase(it2);
	       return_val = true;  
	    }
	    else
	    {
	       return_val = false;  
	    }
	 }
      }
      return return_val;
   }
   const char* params::param::get_value(const hash::path& _path, std::uint32_t _index, std::uint32_t _depth)
   {
      const char* return_val = nullptr;
      std::uint32_t i = _depth++;
      auto it = m_params.find(_path.m_hashes[i]);
      if(it != m_params.end())
      {
	 if(_path.m_name_count > i+1)
	 {
	    return_val = it->second->get_value(_path, _index, _depth);
	 }
	 else if(_index < it->second->m_args.size())
	 {
	    return_val = it->second->m_args[_index].c_str();
	 }
      }
      return return_val;
   }
   const param_args& params::param::get_args(const hash::path& _path, std::uint32_t _depth)
   {
      static param_args g_empty;
      std::uint32_t i = _depth++;
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
      return g_empty;
   }
   bool params::param::set_args(const hash::path& _path, const param_args& _args, std::uint32_t _depth)
   {
      std::uint32_t i = _depth++;
      auto it = m_params.find(_path.m_hashes[i]);
      if(it != m_params.end())
      {
	 if(_path.m_name_count > i+1)
	 {
	    bool success = it->second->set_args(_path, _args, _depth);
	    if(success)
	    {
	       for(auto cb : m_callbacks)
	       {
		  cb->param_cb(m_name.c_str(), m_args);
	       }
	    }
	    return success;
	 }
	 // #todo: define args better, same number of args isn't great.
	 else if(_args.size() == it->second->m_args.size())
	 {
	    it->second->m_args = _args;
	    for (auto cb : it->second->m_callbacks)
	    {
	       cb->param_cb(m_name.c_str(), _args);
	    }
	    return true;
	 }
      }
      return false;
   }
}
