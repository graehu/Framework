#ifndef COMMANDINE_H
#define COMMANDINE_H
#include <cstdint>
#include <stdio.h>
#include <memory>
#include <map>
#include <vector>
#include "hasher.h"

namespace commandline
{
   void parse(int argc, char *argv[]);
   class param
   {
   private:
      friend void commandline::parse(int argc, char *argv[]);
      const char* m_name = nullptr;
      std::vector<const char*> m_args;
   };
   class params
   {
      template<typename T> static bool is_set(T& _topic)
      {
	 auto hash = hash::i32(_topic, sizeof(T)-1);
	 auto it = m_params.find(hash);
	 return it != m_params.end();
      }
   private:
      friend void commandline::parse(int argc, char *argv[]);
      static std::map<std::uint32_t, std::unique_ptr<param>> m_params;
   };
}

#endif//COMMANDLINE_H
