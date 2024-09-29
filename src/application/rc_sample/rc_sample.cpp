#include "rc_sample.h"
#include "../../networking/connection/http_server.h"
#include "../../input/input.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
//STD includes
#include <chrono>
#include <thread>
#include <string>

using namespace fw;

application* application::factory()
{
   static bool do_once = true;
   if(do_once)
   {

      do_once = false;
      return new rc_sample();
   }
   return nullptr;
}

namespace net
{
   class rc_handler : public http_server::handler
   {
   public:
      void get_response(const char* request, http_server::handler::send_callback callback) override
      {
	 // #todo: test this because i don't think it works.
	 if(strcmp(request, "custom") == 0)
	 {
	    callback("you suck", sizeof("you suck"));
	 }
      }
      void ws_response(const char* data, http_server::handler::send_callback /*callback*/) override
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
	    log::debug("read as: x: {} y: {}", lv_dir[0], lv_dir[1]);
	 }
      }
      void ws_send(http_server::handler::send_callback callback) override
      {
	 if(mv_counter % 10 == 0) { callback("ten", sizeof("ten")); }
	 if(mv_counter % 100 == 0) { callback("a hundred", sizeof("a hundred")); }
	 if(mv_counter % 1000 == 0) { callback("a thousand", sizeof("a thousand")); }
	 mv_counter++;
      }
      bool is_ws_handler() override { return true; }
      int mv_counter = 0;
      float x_dir = 0;
      float y_dir = 0;
   };
}

void rc_sample::init()
{
         log::no_topic(R"(
                                                  .__          
_______   ____        ___________    _____ ______ |  |   ____  
\_  __ \_/ ___\      /  ___/\__  \  /     \\____ \|  | _/ __ \ 
 |  | \/\  \___      \___ \  / __ \|  Y Y  \  |_> >  |_\  ___/ 
 |__|    \___  >____/____  >(____  /__|_|  /   __/|____/\___  >
             \/_____/    \/      \/      \/|__|             \/ 
)""\n");

      params::add("rc.port", {"8000"});
      commandline::parse();
      log::topics::add("rc_sample");
}
void rc_sample::run(void)
{
   log::scope topic("rc_sample");
   log::info("----------------");
   int port = 0;
   auto val = params::get_value("rc.port", 0);
   log::info("port set: {}", val);
   port = std::from_string<int>(val);
   input* l_input = input::inputFactory();
   l_input->init();
   log::debug("entering loop");
   net::http_server l_http_server(port);
   net::rc_handler lv_handler;
   commandline::http_handler lv_params_handler;
   l_http_server.add_handler(&lv_handler);
   l_http_server.add_handler(&lv_params_handler);
   bool l_looping = true;
   while(l_looping)
   {
      if(l_input->update()) l_looping = false;
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   delete l_input;
   log::debug("ending loop");
}

void rc_sample::shutdown(){}
