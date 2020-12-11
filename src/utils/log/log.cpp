#include "log.h"
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <queue>
#include <string>
#include "../string_helpers.h"
#include <mutex>
#include <unordered_map>
#include <iostream>

namespace fw
{
   namespace log
   {
      std::mutex g_log_mutex;
      std::map<std::uint32_t, std::unique_ptr<topic> > topics::m_topics;
      std::map<std::thread::id, std::uint32_t> topics::m_thread_topic;
      topic topics::default_topic("log", hash::i32("log"));

      static std::unordered_map<std::string,log::level> const to_level = {
	 {"no_logs", level::e_no_logging},
	 {"debug", level::e_debug},
	 {"info", level::e_info},
	 {"error", level::e_error},
	 {"all", level::e_all},
	 {"warning", level::e_warning},
	 {"fatal", level::e_fatal}
      };
      topic::topic(const char* _topic, uint32_t _hash) :
	 m_name(_topic),
	 m_hash(_hash)
      {
	 m_cb_name = m_name;
	 m_label = m_name;
	 m_label = "[" + m_label + "] ";
	 auto value = params::get_value("log.default.level", 0);
	 if(value != nullptr)
	 {
	    auto new_level = to_level.find(value);
	    if(new_level != to_level.end())
	    {
	       m_level = new_level->second;
	    }
	 }
      }
      void topics::logline(log::level _level, const char* _message, fmt::format_args _args)
      {
	 auto this_id = std::this_thread::get_id();
	 auto thread_topic = m_thread_topic[this_id];
	 if(thread_topic != 0)
	 {
	    m_topics[thread_topic]->logline(_level, _message, _args);
	 }
	 else
	 {
	    default_topic.logline(_level, _message, _args);
	 }
      }
      void topic::logline(level _level, const char* _message, fmt::format_args _args)
      {
	 if (_level <= m_level && _level > e_no_logging)
	 {
#ifndef NO_LOG_LABELS
	    fmt::vprint(m_label + _message, _args);
#else
	    fmt::vprint(_message, _args);
#endif
	    puts("");
	 }
      }
      void topics::log(log::level _level, const char* _message, fmt::format_args _args)
      {
	 auto this_id = std::this_thread::get_id();
	 auto thread_topic = m_thread_topic[this_id];
	 if(thread_topic != 0)
	 {
	    m_topics[thread_topic]->log(_level, _message, _args);
	 }
	 else
	 {
	    default_topic.log(_level, _message, _args);
	 }
      }
      void topic::log(level _level, const char* _message, fmt::format_args _args)
      {
	 if (_level <= m_level && _level > e_no_logging)
	 {
	    fmt::vprint(_message, _args);
	 }
      }
      bool topic::param_cb(const char* _param, const param_args& _args)
      {
	 g_log_mutex.lock();
	 timer cb_timer("topics::param_cb");
	 auto set_level =
	    [this](const param_args& _args)
	    {
	       if(_args.size() > 0)
	       {
		  auto new_level = to_level.find(_args[0]);
		  if(new_level != to_level.end())
		  {
		     printf("[%s] set log level to %s\n", m_name, _args[0].c_str());
		     m_level = new_level->second;
		  }
	       }
	    };
	 bool subscribe = true;
	 std::string log_level = "log.level.";
	 log_level += m_name;
	 auto path = hash::path(log_level.c_str(), log_level.length());
	 if(std::strcmp(_param, "level") == 0)
	 {
	    if(params::subscribe(path, this))
	    {
	       subscribe = false;
	    
	       set_level(params::get_args(path));
	    }
	    else
	    {
	       set_level(_args);
	    }
	 }
	 else
	 {
	    set_level(_args);
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
	    auto path = hash::path(log_level.c_str(), log_level.length());
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
	       params::add({"log.level"}, {});
	       params::subscribe("log.level", _topic);
	    }
	 }
	 g_log_mutex.unlock();
	 return success;
      }
// sets the topic, for the current thread.
      bool topics::set_topic_internal(std::uint32_t _hash)
      {
	 // #todo: locks are a bit wide here.
	 g_log_mutex.lock();
	 bool success = false;
	 auto this_id = std::this_thread::get_id();
	 auto topic_it = topics::m_topics.find(_hash);
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
      void hash_path(const hash::path& _path)
      {
	 printf("--------------\n");
	 printf("directories in path: %d\n", _path.m_name_count);
	 std::uint32_t path_len = _path.m_path_len;
	 printf("0) path: %.*s : len %d : hash %u\n", path_len, _path.m_path, path_len, _path.m_hash);
	 for(std::uint32_t i = 0; i < _path.m_name_count; i++)
	 {
	    std::uint32_t len = _path.m_str_lens[i];
	    printf("%d) %.*s : len %d : hash %u\n", i+1, len, _path.m_names[i], len, _path.m_hashes[i]);
	 }
	 printf("--------------\n");
      }
      timer::timer(const char* _name, bool _condition) :
	 m_condition(_condition)
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
	       std::string message = fmt::format("{} took {} secs", m_name, seconds);
	       debug(message.c_str());
	    }
	    else
	    {
	       std::string message = fmt::format("{} took {} millisecs", m_name, millisecs);
	       debug(message.c_str());
	    }
	 }
      }
   }
}
