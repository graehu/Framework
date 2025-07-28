#include "module_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
import linked_list;

using namespace fw;

application *application::factory() { return new module_sample(); }
void module_sample::init() { commandline::parse(); }
void module_sample::shutdown() {}

// I thought you could implement a template in a module without exposing it's
// implementation. While this in part true, you need to explicitly declare the
// template classes you want to implement like so:
// > linked_list.cpp:6
// > template class LinkedList<int>;
// within the module implementation.

// tl;dr keep modules to 1 file for templates.

void module_sample::run()
{
   log::scope module_sample("module_sample", true);
   LinkedList<int> test;
   test.set_head(10);
   printf("%d\n", *test.get_head());
}
