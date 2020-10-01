#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H

#include <string>
#include <string_view>
#include <sstream>

namespace std
{
   // template<typename T> T from_string(const string_view& str)
   // {
   //    istringstream in(str);
   //    T t;
   //    in >> t;
   //    return t;
   // }
   template<typename T> inline T from_string(const string& str)
   {
      istringstream in(str);
      T t;
      in >> t;
      return t;
   }
   template<typename T> inline string to_string(const T &t)
   {
      ostringstream out;
      out << t;
      return out.str();
   }
   inline bool ends_with(std::string const &fullString, std::string const &ending)
   {
      if (fullString.length() >= ending.length())
      {
   	 return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
      }
      else
      {
   	 return false;
      }
   }
   inline bool ends_with(std::string_view const &fullString, std::string_view const &ending)
   {
      if (fullString.length() >= ending.length())
      {
   	 return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
      }
      else
      {
   	 return false;
      }
   }
}
#endif//STRING_HELPERS_H
