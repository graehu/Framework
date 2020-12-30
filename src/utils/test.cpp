#include "hasher.h"
#include "log/log.h"
#include "params.h"
#include <array>
#include <vector>

using namespace fw;
constexpr hash::string log_hash("log_hash_log");
int main()
{
   log::topics::add(log_hash);
   auto topic = log::scope(log_hash);
   log::debug("test");
   return 0;
}
