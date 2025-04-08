#include "new_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include "../../networking/packet/packet.h"
#include <cstddef>
#include <type_traits>
#include <array>

#include <string_view>
// this code came from here:
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138

template <typename T>
constexpr auto type_name() {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl type_name<";
  suffix = ">(void)";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}

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
   int again;
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

std::vector<struct_a> struct_vas_vec = {};
std::array<struct_a, 8> struct_vas_arr = {};


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

struct MethodHandler
{
   void Call()
   {
      MethodABase* itr = base;
      for(unsigned long i = 0; i < num; i++)
      {
	 if (base != nullptr && ptr != nullptr) { ((itr)->*(ptr))(i); }
	 itr += stride;
      }
   }
   MethodABase* base;
   MethodAPtr ptr;
   unsigned long num = 1;
   size_t stride = sizeof(MethodABase);
};

// this only works for contiguous memory.
// it also assumes the type implements size and operator[]
// might be better to just iterate and store array of base ptrs up to some max.
template <template <typename, typename...> class c, typename t, typename... ts>
MethodHandler GetMethodAHandler(c<t, ts...>& in)
{
   log::info("Generate array method handler");
   MethodHandler out = { (MethodABase*)&in[0], MethodAHelper<t>::GetPtr(), in.size(), sizeof(t) };
   return out;
}

template <template <typename, size_t, typename...> class c, typename t, size_t N, typename... ts>
MethodHandler GetMethodAHandler(c<t, N, ts...>& in)
{
   log::info("Generate array method handler with int");
   MethodHandler out = { (MethodABase*)&in[0], MethodAHelper<t>::GetPtr(), in.size(), sizeof(t) };
   return out;
}

// this works for anything.
template <typename t>
MethodHandler GetMethodAHandler(t& in)
{
   log::info("Generate base method handler");
   MethodHandler out = { (MethodABase*)&in, MethodAHelper<t>::GetPtr() };
   return out;
}

void new_sample::run()
{
   log::scope new_sample("new_sample", true);
   {
      struct_vas_vec.push_back({0,20,9});
      struct_vas_vec.push_back({0,30,0});
      struct_vas_arr[0] = {0,20,9};
      struct_vas_arr[7] = {0,20,9};
      struct_a test_a;
      struct_b test_b;
      struct_c test_c;
      MethodAHelper<decltype(test_a)>::Call(test_a); // calls the correct function
      // MethodAHelper<decltype(test_b)>::Call(test_b); // this is a compile error, wrong declaration
      MethodAHelper<decltype(test_c)>::Call(test_c); // handles missing declaration
      test_a.data = 10;
      GetMethodAHandler(test_a).Call(); // calls the correct function
      GetMethodAHandler(test_b).Call(); // calls incorrect fuction, kind of.
      GetMethodAHandler(test_c).Call(); // handles nullptr
      GetMethodAHandler(struct_vas_vec).Call(); // handles vectors without knowing the type.
      GetMethodAHandler(struct_vas_arr).Call();
   }
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
