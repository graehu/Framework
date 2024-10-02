#include "new_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include "../../networking/packet/packet.h"
#include <cstdint>
#include <string>
#include <cstring>
#include <type_traits>

using namespace fw;

application *application::factory() { return new new_sample(); }
void new_sample::init() { commandline::parse(); }
void new_sample::shutdown() { }


struct struct_a
{
   int other;
   void method_a(int test) { log::info("this function will be called {} <-> {}", data, test); }
   void method_b(int test) { log::info("this wont be called {}", test); }
   int data;
};

struct struct_b
{
   void method_a() { log::info("I'll get called even though I don't have params"); }
   void method_b() { log::info("this wont be called"); }
};

struct struct_c
{
   void method_b() { log::info("this wont be called"); }
};


struct MethodABase {}; // concrete type to call funcs on
typedef void (MethodABase::* MethodAPtr)(int);
template <typename _Type>
struct MethodAHelper
{
   static void Call(_Type& t) { Call(t, typename Has::type()); }
   static MethodAPtr GetPtr() { return GetPtr(typename Has::type()); }
   private:
   struct Has
   {
      template<typename _Any> static std::true_type Check(decltype(&_Any::method_a));
      template<typename _Any> static std::false_type Check(...);
      typedef std::is_same<std::true_type, decltype(Check<_Type>(nullptr))> type;
   };
   static void Call(_Type& t, std::true_type) { t.method_a(1); }
   static void Call(_Type&, std::false_type)
   {
      log::error("not calling function for some type");
   }
   static MethodAPtr GetPtr(std::true_type){ return (MethodAPtr)&_Type::method_a; }
   static MethodAPtr GetPtr(std::false_type){ return nullptr; }
};
void CallMethodAPtr(MethodABase* base, MethodAPtr ptr)
{
   if (base != nullptr && ptr != nullptr) { (base->*ptr)(1); }
}

void new_sample::run()
{
   log::scope new_sample("new_sample", true);
   {
      struct_a test_a;
      struct_b test_b;
      struct_c test_c;
      MethodAHelper<decltype(test_a)>::Call(test_a); // calls the correct function
      // MethodAHelper<decltype(test_b)>::Call(test_b); // this is a compile error, wrong declaration
      MethodAHelper<decltype(test_c)>::Call(test_c); // handles missing declaration
      test_a.data = 10;
      auto ptr = MethodAHelper<decltype(test_a)>::GetPtr();
      CallMethodAPtr((MethodABase*)&test_a, ptr); // calls the correct function
      ptr = MethodAHelper<decltype(test_b)>::GetPtr();
      CallMethodAPtr((MethodABase*)&test_b, ptr); // calls incorrect fuction, kind of.
      ptr = MethodAHelper<decltype(test_c)>::GetPtr();
      CallMethodAPtr((MethodABase*)&test_c, ptr); // handles nullptr
      {
	 typedef net::NewPacket<32> test_packet;
	 test_packet test; test.Clear();
	 log::info("test packet size should be zero == {}", test.GetSize());
      }
      {
	 typedef net::NewPacket<32, short> test_packet;
	 test_packet test; test.Clear();
	 log::info("test packet size should be 2 == {}", test.GetSize());
      }
   }
}
