#include <string>
#include <sstream>

namespace std
{
   template<typename T> T from_string(const string &str)
   {
      istringstream in(str);
      T t;
      in >> t;
      return t;
   }
   template<typename T> string to_string(const T &t)
   {
      ostringstream out;
      out << t;
      return out.str();
   }  
}
