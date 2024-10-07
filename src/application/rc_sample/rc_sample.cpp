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
   return new rc_sample();
}

namespace net
{
   const char lipsum256[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec viverra nisl sit amet orci imperdiet, quis cursus risus eleifend. Sed eget augue quis diam fringilla pulvinar. Curabitur vulputate libero id arcu mollis, eget imperdiet dui mattis. Donec donec.";
   const char lipsum4096[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec a eleifend nisl. Sed sed augue eu dolor blandit tempor hendrerit et sapien. Donec sed bibendum libero, bibendum efficitur arcu. Praesent sed dapibus quam. Integer iaculis massa quis diam porttitor porttitor. Cras ex nisi, ornare ac elementum vitae, imperdiet id arcu. Quisque quis eleifend nibh, id facilisis sem. Cras at tempus velit, lobortis iaculis ipsum. Ut cursus, felis a hendrerit condimentum, mauris neque rhoncus elit, eleifend mattis arcu mauris in ante. Nullam sollicitudin mollis urna, et commodo purus volutpat id. Nunc sagittis pharetra nibh, ut finibus lacus vulputate eu. Curabitur id sollicitudin elit, ut facilisis massa. Curabitur felis tortor, accumsan eu lacinia ac, eleifend venenatis nisi. Maecenas varius nisi eget consequat blandit. Fusce et leo lacus. Nulla porttitor elit eu molestie viverra. Integer suscipit facilisis libero in blandit. Integer ut egestas mauris, et viverra magna. Sed cursus lectus vitae euismod sollicitudin. Aenean consectetur sed lorem non accumsan. Phasellus hendrerit mauris ligula, id vehicula libero consectetur a. Duis euismod finibus tempor. Duis vitae enim ipsum. Duis sed diam nisi. Donec a cursus augue. Sed sed sapien semper, feugiat enim in, euismod purus. Maecenas quam leo, rhoncus non placerat id, faucibus quis lorem. Nam mollis dolor vestibulum fermentum sagittis. Sed ac sem nibh. Etiam lacinia dolor mauris, a semper sapien eleifend vel. Morbi ac maximus lorem, dapibus ultricies nulla. In ac justo fringilla, pretium felis in, consectetur enim. In volutpat non risus porttitor sagittis. Maecenas erat tellus, pulvinar sed mauris ut, mollis euismod eros. Aliquam erat volutpat. In hac habitasse platea dictumst. Pellentesque orci urna, bibendum et tempor in, malesuada et tellus. Integer facilisis aliquam tellus, eu posuere sem tempor non. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Maecenas odio orci, cursus cursus lorem id, elementum laoreet lacus. In varius quam lectus, at lobortis felis consectetur a. Aliquam et ipsum tristique, ullamcorper turpis at, fermentum est. Nulla facilisi. Pellentesque ut diam dictum, tristique nisi ac, ultrices lectus. Curabitur vitae lectus eget dolor eleifend malesuada vel ullamcorper libero. Aenean posuere est et lacinia condimentum. Morbi dignissim lectus quis massa faucibus pulvinar. Ut elementum tortor sit amet tristique scelerisque. Maecenas erat neque, scelerisque non leo in, ornare lobortis nulla. Pellentesque quis est mollis, accumsan magna mollis, ultrices enim. Proin tempus finibus diam, nec bibendum ex pharetra sit amet. Sed sem tellus, mollis nec metus at, sodales faucibus arcu. Nam pellentesque, mi ac finibus maximus, nunc odio tincidunt nisl, at elementum arcu mi sed lacus. Aliquam ultricies quam augue, in tristique massa ultrices et. Pellentesque efficitur nisi nisl, id malesuada nunc ornare vitae. Vestibulum urna justo, laoreet sed nibh sed, vestibulum varius elit. Cras vel pretium mauris. Sed condimentum enim ac magna malesuada, vitae ultrices est rutrum. Cras in mi et velit sollicitudin convallis. Nunc fringilla, dui tempus vulputate venenatis, leo felis porta lectus, in ultrices nisi lectus eget libero. Aliquam congue eget lorem lacinia accumsan. Curabitur fringilla, nulla quis condimentum eleifend, libero ipsum rutrum sapien, vel lobortis quam magna eget mi. Ut nunc purus, sodales ac malesuada sed, consequat eget tortor. Proin ut mattis diam. Nulla convallis augue id metus iaculis scelerisque. Nulla facilisi. Vivamus condimentum arcu efficitur, blandit ipsum et, consequat est. Donec placerat dolor in turpis mollis, in elementum tellus porttitor. Vestibulum porta mollis massa ut ultricies. Integer eu elit lobortis, varius elit ut, iaculis ipsum. Nulla vulputate convallis scelerisque. Integer nunc leo, tristique ut bibendum ac, faucibus sit amet enim. Vivamus efficitur fringilla pulvinar. Praesent a eros vehicula, tempus tellus ut, varius leo. Etiam tempus dolor ut tellus pellentesque fringilla. Fusce scelerisque aliquam libero.";
   class rc_handler : public http_server::handler
   {
   public:
      void get_response(std::string_view request, http_server::handler::send_callback callback) override
      {
	 // #todo: test this because i don't think it works.
	 if(request.compare("custom") == 0)
	 {
	    callback("you suck", sizeof("you suck"));
	 }
      }

      void ws_response(const char* data, http_server::handler::ws_send_callback /*callback*/) override
      {
	 // #todo: does this pattern make sense? websockets are less of a response driven protocol
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
      void ws_send(http_server::handler::ws_send_callback callback) override
      {
	 if(mv_counter == 0) { callback(lipsum256, sizeof(lipsum256), true); }
	 if(mv_counter == 1) { callback(lipsum4096, sizeof(lipsum4096), true); }
	 if(mv_counter % 10 == 0) { callback("ten", sizeof("ten"), true); }
	 if(mv_counter % 100 == 0) { callback("a hundred", sizeof("a hundred"), true); }
	 if(mv_counter % 1000 == 0) { callback("a thousand", sizeof("a thousand"), true); }
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
