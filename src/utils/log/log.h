#ifndef LOG_H
#define LOG_H
#include "../hasher.h"
#include "../params.h"
#include <string>
#include <map>
#include <memory>
#include <thread>
#include <chrono>
#include "fmt/core.h"

#define __Q(x) #x
#define QUOTE(x) __Q(x)

namespace fw
{
   namespace log
   {
      // #todo: consider a namespace with static const uints?
      enum level {  e_no_logging, e_fatal, e_error, e_warning, e_info, e_debug, e_all };
      // 
      class topic final : private params::callback
      {
      public:
	 void logline(level _level, const char* _message, fmt::format_args _args);
	 void log(level _level, const char* _message, fmt::format_args _args);
	 const char* name() const { return m_name; }
	 std::uint32_t hash() const { return m_hash; }
	 level m_level = e_info;
      private:
	 topic(const char* _topic, uint32_t _hash);
	 bool param_cb(const char* _param_name, const param_args& _args) final;
	 const char* m_name;
	 std::string m_label;
	 std::uint32_t m_hash;
	 friend class topics;
	 friend class scope;
      };
      
      // global accessor for topics, used by log::info etc.
      // usage: log::topics::add("my_topic");
      class topics
      {
      public:
	 static void logline(level _level, const char* _message, fmt::format_args _args);
	 static void log(level _level, const char* _message, fmt::format_args _args);
	 template<level L, typename... Args>
	 void static logline(const char* _message, Args... _args)
	 {
	    auto store = fmt::make_format_args<fmt::format_context, Args...>(_args...);
	    topics::logline(L, _message, store);
	 }
	 template<level L, typename... Args>
	 void static log(const char* _message, Args... _args)
	 {
	    auto store = fmt::make_format_args<fmt::format_context, Args...>(_args...);
	    topics::log(L, _message, store);
	 }
	 static bool add(const hash::string& _topic)
	 {
	    return topics::add_topic_internal(new topic(_topic.m_literal, _topic.m_hash));
	 }
	 static bool set(const hash::string& _topic, bool _add_topic = false)
	 {
	    
	    bool result = topics::set_topic_internal(_topic.m_hash);
	    if(!result && _add_topic)
	    {
	       topics::add(_topic);
	       result = topics::set_topic_internal(_topic.m_hash);
	    }
	    return result;
	 }
	 static const char* get();
	 static uint32_t hash();
	 static bool set_level(const hash::string& _topic, log::level _level)
	 {
	    return topics::set_level_internal(_topic.m_hash, _level);
	 }
	 static log::level get_level(const hash::string& _topic)
	 {
	    return topics::get_level_internal(_topic.m_hash);
	 }
	 static log::level get_level()
	 {
	    return topics::get_level_internal();
	 }
      private:
	 friend class scope;
	 static bool add_topic_internal(topic* _topic);
	 static bool set_topic_internal(std::uint32_t _hash);
	 static bool set_level_internal(std::uint32_t _hash, log::level _level);
	 static log::level get_level_internal(std::uint32_t _hash);
	 static log::level get_level_internal();
	 static std::map<std::uint32_t, std::unique_ptr<topic>> m_topics;
	 static std::map<std::thread::id, std::uint32_t > m_thread_topic;
	 static topic default_topic;
      };
      
      // used to set the current scopes log topic, will be reset on destruction.
      // usage: log::scope topic("my_log");
      class scope
      {
      public:
	 scope(const hash::string& _topic, bool _add_topic = false)
	 {
	    m_last_hash = topics::hash();
	    topics::set(_topic, _add_topic);
	 }
	 ~scope()
	 {
	    topics::set_topic_internal(m_last_hash);
	 }
      private:
	 uint32_t  m_last_hash;
      };
      
      // used to time the current scope, ends on destruction.
      // usage:: log::timer("name_of_section");
      class timer
      {
      public:
	 timer(const char* _name, bool _condition = true);
	 ~timer();
      private:
	 std::chrono::high_resolution_clock m_clock;
	 std::chrono::_V2::system_clock::time_point m_start;
	 const char* m_name = nullptr;
	 bool m_condition = true;
      };
      
      // #todo: move type logging into a different file
      void hash_path(const hash::path& _path);
      
      // #todo: find a way to do this automatically, one for every entery in levels enum
      template<typename... Args>
      void fatal(const char* _message, Args... _args)
      {
	 topics::logline<e_fatal>(_message, _args...);
	 __builtin_trap();
      }
      template<typename... Args>
      void error(const char* _message, Args... _args)
      {
	 topics::logline<e_error>(_message, _args...);
      }
      template<typename... Args>
      void warn(const char* _message, Args... _args)
      {
	 topics::logline<e_warning>(_message, _args...);
      }
      template<typename... Args>
      void debug(const char* _message, Args... _args)
      {
	 topics::logline<e_debug>(_message, _args...);
      }
      template<typename... Args>
      void info(const char* _message, Args... _args)
      {
	 topics::logline<e_info>(_message, _args...);
      }
      template<typename... Args>
      void debug_inline(const char* _message, Args... _args)
      {
	 topics::log<e_debug>(_message, _args...);
      }
      template<typename... Args>
      void info_inline(const char* _message, Args... _args)
      {
	 topics::log<e_info>(_message, _args...);
      }
      template<typename... Args>
      void no_topic(const char* _message, Args... _args)
      {
	 auto store = fmt::make_format_args<fmt::format_context, Args...>(_args...);
	 fmt::vprint(_message, store);
      }
   }
}
#endif//LOG_H
