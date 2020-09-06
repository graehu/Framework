#include "param_sample.h"
#include "../../utils/string_helpers.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include <string>
#include <unordered_map>

static std::unordered_map<std::string,log::level> const to_level = {
      {"no_logs", log::level::e_no_logging},
      {"debug", log::level::e_debug},
      {"info", log::level::e_info},
      {"error", log::level::e_error},
      {"macro", log::level::e_macro},
      {"warning", log::level::e_warning}
   };

application* application::mf_factory()
{
   return new param_sample();
}

void param_sample::mf_run(void)
{
   log::topics::add("param_sample");
   auto scope = log::scope("param_sample");
   log::info("-----------------------");
   log::info("-----------------------");
   params::add("log.level.param_sample", {"debug"});
   {
      log::timer loop_adds("1000000 param adds");
      for(int i = 0; i < 1000000; i++)
      {
      	 std::string num = std::to_string(i);
      	 num = "log.level."+num;
      	 auto path = hash::make_path(num.c_str(), num.length());
      	 params::add(path, {"debug"});
      }
   }
   {
      log::timer loop_adds("10000 param adds");
      for(int i = 1000000; i < 1010000; i++)
      {
	 std::string num = std::to_string(i);
	 num = "log.level."+num;
	 auto path = hash::make_path(num.c_str(), num.length());
	 params::add(path, {"debug"});
      }
   }
   {
      log::timer loop_sets("10000 param sets");
      for(int i = 0; i < 10000; i++)
      {
	 
	 std::string num = std::to_string(i);
	 num = "log.level."+num;
	 auto path = hash::make_path(num.c_str(), num.length());
	 params::set_args(path, {"debug"});
      }
   }
   {
      log::timer loop_gets("10000 param get_args");
      log::level _level;
      for(int i = 0; i < 10000; i++)
      {
	 std::string num = std::to_string(i);
	 num = "log.level."+num;
	 auto path = hash::make_path(num.c_str(), num.length());
	 auto value = params::get_args(path);
	 _level = to_level.find(value[0])->second;
      }
   }
   {
      log::timer loop_gets("10000 param gets");
      log::level _level;
      for(int i = 0; i < 10000; i++)
      {
	 std::string num = std::to_string(i);
	 num = "log.level."+num;
	 auto path = hash::make_path(num.c_str(), num.length());
	 auto value = params::get_value(path, 0);
	 _level = to_level.find(value)->second;
      }
   }
}
