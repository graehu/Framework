#include "rc_sample.h"
#include "../../networking/connection/http_server.h"
#include "../../input/input.h"
#include "../../utils/log/log.h"
#include "../../utils/commandline.h"
//STD includes
#include <chrono>
#include <cstdio>
#include <thread>
#include <iostream>
#include <cstring>
#include <string>


application* application::mf_factory()
{
   return new rc_sample();
}
namespace net
{
   class rc_handler : public http_server::handler
   {
   public:
      void mf_get_response(const char* request, http_server::handler::send_callback callback) override
      {
	 //todo: test this because i don't think it works.
	 if(strcmp(request, "custom") == 0)
	 {
	    callback("you suck", sizeof("you suck"));
	 }
      }
      void mf_ws_response(const char* data, http_server::handler::send_callback callback) override
      {
	 std::string lv_data(data);
	 const char lv_dir_label[] = { "d:[" };
	 auto lv_dir_start = lv_data.find(lv_dir_label);
	 if (lv_dir_start != std::string::npos)
	 {
	    lv_dir_start += sizeof(lv_dir_label)-1;
	    auto lv_len = lv_data.find(']', lv_dir_start) - lv_dir_start;
	    std::string lv_floats = lv_data.substr(lv_dir_start, lv_len);
	    const char delimiter[] = { "," };
	    std::size_t lv_pos = 0;
	    std::string token;
	    std::vector<float> lv_dir;
	    while ((lv_pos = lv_floats.find(delimiter)) != std::string::npos)
	    {
	       token = lv_floats.substr(0, lv_pos);
	       lv_floats.erase(0, lv_pos + sizeof(delimiter)-1);
	       lv_dir.push_back(std::stof(token));
	    }
	    lv_dir.push_back(std::stof(lv_floats));
	    log::info("read as: x: %f y: %f", lv_dir[0], lv_dir[1]);
	 }
      }
      void mf_ws_send(http_server::handler::send_callback callback) override
      {
	 if(mv_counter % 10 == 0) { callback("ten", sizeof("ten")); }
	 if(mv_counter % 100 == 0) { callback("a hundred", sizeof("a hundred")); }
	 if(mv_counter % 1000 == 0) { callback("a thousand", sizeof("a thousand")); }
	 mv_counter++;
      }
      const char* mf_get_response_root_dir() const override
      {
	 return "";
      }
      int mv_counter = 0;
      float x_dir = 0;
      float y_dir = 0;
   };
}
void rc_sample::mf_run(void)
{
   log::no_topic(R"(
                                                  .__          
_______   ____        ___________    _____ ______ |  |   ____  
\_  __ \_/ ___\      /  ___/\__  \  /     \\____ \|  | _/ __ \ 
 |  | \/\  \___      \___ \  / __ \|  Y Y  \  |_> >  |_\  ___/ 
 |__|    \___  >____/____  >(____  /__|_|  /   __/|____/\___  >
             \/_____/    \/      \/      \/|__|             \/ 
)""\n");
   commandline::parse(commandline::arg_count, commandline::arg_variables);
   if(commandline::params::is_set("something"))
   {
      log::info("something is set!");
      auto val = commandline::params::get<int>("something", 0);
      if(val.first)
      {
	 log::info("it's set to: %d", val.second);
      }
   }
   log::topics::add("rc_sample");
   log::topics::set("rc_sample");
   input* l_input = input::inputFactory();
   l_input->init();
   net::http_server l_http_server(8000);
   net::rc_handler lv_handler;
   l_http_server.mf_set_handler(&lv_handler);
   log::info("entering rc_sample loop");
   bool l_looping = true;
   while(l_looping)
   {
      if(l_input->update()) l_looping = false;
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   delete l_input;
   log::info("ending rc_sample loop");
}
