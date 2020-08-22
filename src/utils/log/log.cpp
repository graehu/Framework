#include "log.h"
#include <cstdarg>
#include <cstring>
#include <memory>
#include <string>
#include <mutex>
#include <iostream>

namespace log
{
   std::mutex g_log_mutex;
   std::map<const char*, std::unique_ptr<topic> > topics::m_topics;
   std::map<std::thread::id, const char*> topics::m_thread_topic;
   void topics::log(level _level, const char* _message, std::va_list args)
   {
      auto this_id = std::this_thread::get_id();
      const char* thread_topic = m_thread_topic[this_id];
      if(thread_topic != nullptr)
      {
	 m_topics[thread_topic]->log(_level, _message, args);
      }
   }
   void topic::log(level _level, const char* _message, va_list args)
   {
      if (_level <= m_level && _level > e_no_logging)
      {
	 printf("\n[%s] ", m_topic.c_str());
	 vprintf(_message, args);
      }
   }
// adds a topic to the global list.
   bool add_topic(const char* _topic)
   {
      bool success = false;
      std::unique_ptr<topic> emplace_topic(new topic(_topic));
      g_log_mutex.lock();
      success = topics::m_topics.emplace(emplace_topic->literal(), emplace_topic.release()).second;
      g_log_mutex.unlock();
      return success;
   }
// sets the topic, for the current thread.
   bool set_topic(const char* _topic)
   {
      //todo: locks are a bit wide here.
      g_log_mutex.lock();
      bool success = false;
      auto this_id = std::this_thread::get_id();
      auto topic_it = topics::m_topics.end();
      for(auto it = topics::m_topics.begin(); it != topics::m_topics.end(); it++)
      {
	 if(std::strcmp(it->second->literal(), _topic) == 0)
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
	    thread_it->second = topic_it->second->literal();
	 }
	 else
	 {
	    topics::m_thread_topic.insert({this_id, topic_it->second->literal()});
	 }
	 success = true;
      }
      g_log_mutex.unlock();
      return success;
   }
   // sets the log level of a topic
   bool set_level(const char* _topic, level _level)
   {
      //don't think this needs a lock, but we'll see I guess?
      auto this_id = std::this_thread::get_id();
      const char* thread_topic = topics::m_thread_topic[this_id];
      if(thread_topic != nullptr)
      {
	 topics::m_topics[thread_topic]->m_level = _level;
	 return true;
      }
      return false;
   }
   // gets the topic, for the current thread.
   const char* get_topic()
   {
      auto this_id = std::this_thread::get_id();
      return topics::m_thread_topic[this_id];
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
