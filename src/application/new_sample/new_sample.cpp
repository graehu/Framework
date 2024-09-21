#include "new_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include <string>
#include <cstring>
#include <type_traits>

using namespace fw;

application* application::factory()
{
   return new new_sample();
}

struct struct_a
{
   void method_a() { log::info("this function will be called"); }
};

struct struct_b
{
   void method_b() { log::info("this wont be called"); }
};


template <typename _Type>
struct CallIfMethodA
{
   static void method_a(_Type& t)
   {
      method_a(t, typename HasMethod::type());
   }
   private:
   struct HasMethod
   {
      template<typename _Any> static std::true_type Check(decltype(&_Any::method_a));
      template<typename _Any> static std::false_type Check(...);
      typedef std::is_same<std::true_type, decltype(Check<_Type>(nullptr))> type;
   
   };
   static void method_a(_Type& t, std::true_type) { t.method_a(); }
   static void method_a(_Type&, std::false_type)
   {
      log::error("not calling function for some type");
   }
};

#define call_method(x) CallIfMethodA<decltype(x)>::method_a(x);

void new_sample::run()
{
   commandline::parse();
   log::scope new_sample("new_sample", true);
   log::timer total("total");
   {
      struct_a test_a;
      struct_b test_b;
      call_method(test_a);
      call_method(test_b);
   }
}
