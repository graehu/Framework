#include "log.h"
#include <bits/stdint-uintn.h>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <string>
#include <mutex>
#include <iostream>

namespace log
{
   std::mutex g_log_mutex;
   std::map<std::uint32_t, std::unique_ptr<topic> > topics::m_topics;
   std::map<std::thread::id, std::uint32_t> topics::m_thread_topic;
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
	 //todo: put the \n on the end of the string.
	 //      wrong way for ease of use atm.
	 printf("\n[%s] ", m_name);
	 vprintf(_message, args);
      }
   }
// adds a topic to the global list.
   bool topics::add_topic_internal(topic* _topic)
   {
      bool success = false;
      g_log_mutex.lock();
      auto hash = _topic->hash();
      success = topics::m_topics.emplace(hash, _topic).second;
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
      topics::log(log::e_info, _message, args);
      va_end(args);
   }
// log severity warning.
   void warning(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_warning, _message, args);
      va_end(args);
   }
// log severity error.
   void error(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_error, _message, args);
      va_end(args);
   }
// log severity debug.
   void debug(const char* _message, ...)
   {
      va_list args;
      va_start(args, _message);
      topics::log(log::e_debug, _message, args);
      va_end(args);
   }
   void macro(const char* _message, ...)
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
}
