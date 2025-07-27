#include "module_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
// import "linked_list.ixx";
import linked_list;


using namespace fw;

application *application::factory() { return new module_sample(); }
void module_sample::init() { commandline::parse(); }
void module_sample::shutdown() {}

void module_sample::run()
{
   log::scope module_sample("module_sample", true);
}
