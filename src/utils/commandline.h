#ifndef COMMANDINE_H
#define COMMANDINE_H
#include <cstdint>
#include <stdio.h>
#include <memory>
#include <map>
#include <vector>
#include "string_helpers.h"
#include "hasher.h"

namespace commandline
{
   extern int arg_count;
   extern char** arg_variables;
   void parse(int argc, char *argv[]);
   class params
   {
   public:
      template<typename T> static bool is_set(T& _flag)
      {
	 auto hash = hash::i32(_flag, sizeof(T)-1);
	 auto it = m_params.find(hash);
	 return it != m_params.end();
      }
      template<typename R, typename T> static std::pair<bool, R> get(T& _flag, int _index)
      {
	 auto hash = hash::i32(_flag, sizeof(T)-1);
	 auto it = m_params.find(hash);
	 bool success = false;
	 if(it != m_params.end())
	 {
	    if(_index < it->second->m_args.size())
	    {
	       return std::pair<bool, R>(true, std::from_string<R>(it->second->m_args[_index]));
	    }
	 }
	 return std::pair<bool, R>(false, R());
      }
   private:
      class param
      {
      public:
	 friend void commandline::parse(int argc, char *argv[]);
	 const char* m_name = nullptr;
	 std::vector<const char*> m_args;
      };
      friend void commandline::parse(int argc, char *argv[]);
      static std::map<std::uint32_t, std::unique_ptr<param>> m_params;
   };
}

#endif//COMMANDLINE_H
