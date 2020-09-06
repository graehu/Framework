#include "log.h"
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <string>
#include "../string_helpers.h"
#include <mutex>
#include <unordered_map>
#include <iostream>

namespace log
{
   std::mutex g_log_mutex;
   std::map<std::uint32_t, std::unique_ptr<topic> > topics::m_topics;
   std::map<std::thread::id, std::uint32_t> topics::m_thread_topic;

   static std::unordered_map<std::string,log::level> const to_level = {
      {"no_logs", level::e_no_logging},
      {"debug", level::e_debug},
      {"info", level::e_info},
      {"error", level::e_error},
      {"macro", level::e_macro},
      {"warning", level::e_warning}
   };
   void topics::logline(log::level _level, const char* _message, std::va_list args)
   {
      auto this_id = std::this_thread::get_id();
      auto thread_topic = m_thread_topic[this_id];
      if(thread_topic != 0)
      {
	 m_topics[thread_topic]->logline(_level, _message, args);
      }
   }
   void topic::logline(level _level, const char* _message, va_list args)
   {
      if (_level <= m_level && _level > e_no_logging)
      {
	 printf("[%s] ", m_name);
	 vprintf(_message, args);
	 printf("\n");
      }
   }
   void topics::log(log::level _level, const char* _message, std::va_list args)
   {
      auto this_id = std::this_thread::get_id();
      auto thread_topic = m_thread_topic[this_id];
      if(thread_topic != 0)
      {
	 m_topics[thread_topic]->log(_level, _message, args);
      }
   }
   void topic::log(level _level, const char* _message, va_list args)
   {
      if (_level <= m_level && _level > e_no_logging)
      {
	 vprintf(_message, args);
      }
   }
   bool topic::param_cb(const char* _param, param_args _args)
   {
      g_log_mutex.lock();
      timer cb_timer("topics::param_cb");
      bool subscribe = true;
      std::string log_level = "log.level.";
      log_level += m_name;
      auto path = hash::make_path(log_level.c_str(), log_level.length());
      if(std::strcmp(_param, "level") == 0)
      {
	 if(params::subscribe(path, this))
	 {
	    subscribe = false;
	    _args = params::get_args(path);
	 }
      }
      if(_args.size() > 0)
      {
	 auto new_level = to_level.find(_args[0]);
	 if(new_level != to_level.end())
	 {
	    printf("[%s] set log level to %s\n", m_name, _args[0].c_str());
	    m_level = new_level->second;
	 }
      }
      g_log_mutex.unlock();
      return subscribe;
   }
// adds a topic to the global list.
   bool topics::add_topic_internal(topic* _topic)
   {
      bool success = false;
      g_log_mutex.lock();
      auto hash = _topic->hash();
      auto it = topics::m_topics.find(hash);
      if(it == topics::m_topics.end())
      {
	 success = topics::m_topics.emplace(hash, _topic).second;
	 std::string log_level = "log.level.";
	 log_level += _topic->m_name;
	 auto path = hash::make_path(log_level.c_str(), log_level.length());
	 auto value = params::get_value(path, 0);
	 if(value != nullptr)
	 {
	    auto new_level = to_level.find(value);
	    if(new_level != to_level.end())
	    {
	       _topic->m_level = new_level->second;
	    }
	    params::subscribe(path, _topic);
	 }
	 else
	 {
	    params::add("log.level", {});
	    params::subscribe("log.level", _topic);
	 }
      }
      g_log_mutex.unlock();
      return success;
   }
// sets the topic, for the current thread.
   bool topics::set_topic_internal(std::uint32_t _hash)
   {
      //todo: locks are a bit wide here.
      g_log_mutex.lock();
      bool success = false;
      auto this_id = std::this_thread::get_id();
      auto topic_it = topics::m_topics.end();
      for(auto it = topics::m_topics.begin(); it != topics::m_topics.end(); it++)
      {
	 if(it->second->hash() == _hash)
	 {
	    topic_it = it;
	    break;
	 }
      }
      if(topic_it  != topics::m_topics.end())
      {
	 auto thread_it = topics::m_thread_topic.find(this_id);
	 if(thread_it != topics::m_thread_topic.end())
	 {
	    thread_it->second = topic_it->second->hash();
	 }
	 else
	 {
	    topics::m_thread_topic.insert({this_id, topic_it->second->hash()});
	 }
	 success = true;
      }
      g_log_mutex.unlock();
      return success;
   }
   // sets the log level of a topic
   bool topics::set_level_internal(std::uint32_t _hash, log::level _level)
   {
      //don't think this needs a lock, but we'll see I guess?
      auto it = topics::m_topics.find(_hash);
      if(it != topics::m_topics.end())
      {
	 it->second->m_level = _level;
	 return true;
      }
      return false;
   }
   log::level topics::get_level_internal(std::uint32_t _hash)
   {
      auto it = topics::m_topics.find(_hash);
      if(it != topics::m_topics.end())
      {
	 return it->second->m_level;
      }
      return e_no_logging;
   }
   log::level topics::get_level_internal()
   {
      auto this_id = std::this_thread::get_id();
      auto hash = topics::m_thread_topic[this_id];
      return get_level_internal(hash);
   }
   // gets the topic, for the current thread.
   const char* topics::get()
   {
      auto this_id = std::this_thread::get_id();
      auto hash = topics::m_thread_topic[this_id];
      auto it = topics::m_topics.find(hash);
      if(it != topics::m_topics.end())
      {
	 return it->second->name();
      }
      return nullptr;
   }
   std::uint32_t topics::hash()
   {
      auto this_id = std::this_thread::get_id();
      auto hash = topics::m_thread_topic[this_id];
      auto it = topics::m_topics.find(hash);
      if(it != topics::m_topics.end())
      {
	 return it->second->hash();
      }
      return 0;
   }
   // log severity info.
   void info(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::logline(log::e_info, _message, args);
      va_end(args);
   }
// log severity warning.
   void warning(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::logline(log::e_warning, _message, args);
      va_end(args);
   }
// log severity error.
   void error(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::logline(log::e_error, _message, args);
      va_end(args);
   }
// log severity debug.
   void debug(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::logline(log::e_debug, _message, args);
      va_end(args);
   }
   void macro(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::logline(log::e_debug, _message, args);
      va_end(args);
   }
   void info_inline(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_info, _message, args);
      va_end(args);
   }
// log severity warning.
   void warning_inline(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_warning, _message, args);
      va_end(args);
   }
// log severity error.
   void error_inline(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_error, _message, args);
      va_end(args);
   }
// log severity debug.
   void debug_inline(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_debug, _message, args);
      va_end(args);
   }
   void macro_inline(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_debug, _message, args);
      va_end(args);
   }
   void no_topic(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      vprintf(_message, args);
      va_end(args);
   }
   void hash_path(hash::path&& _path)
   {
      printf("--------------\n");
      printf("directories in path: %d\n", _path.m_name_count);
      std::uint32_t path_len = _path.m_path_len;
      printf("0) path: %.*s : len %d : hash %u\n", path_len, _path.m_path, path_len, _path.m_hash);
      for(int i = 0; i < _path.m_name_count; i++)
      {
	 std::uint32_t len = _path.m_str_lens[i];
	 printf("%d) %.*s : len %d : hash %u\n", i+1, len, _path.m_names[i], len, _path.m_hashes[i]);
      }
      printf("--------------\n");
   }
   timer::timer(const char* _name, bool _condition)
   {
      m_name = _name;
      m_start = m_clock.now();
   }
   timer::~timer()
   {
      if(m_condition)
      {
	 auto time = m_clock.now()-m_start;
	 auto millisecs = std::chrono::duration<float, std::milli>(time).count();
	 if(millisecs > 1000)
	 {
	    auto seconds = std::chrono::duration<float>(time).count();
	    debug("%s took %f secs", m_name, seconds);
	 }
	 else
	 {
	    debug("%s took %f millisecs", m_name, millisecs);
	 }
      }
   }
}
