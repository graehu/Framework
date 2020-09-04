#ifndef LOG_H
#define LOG_H
#include "../hasher.h"
#include "../params.h"
#include <cstdarg>
#include <cstdint>
#include <map>
#include <memory>
#include <thread>
#include <iosfwd>


#define log_int(variable)
#define log_uint(variable) 
#define log_float(variable) 
#define log_str(variable)
#define log_message(message)


namespace log
{
   enum level {  e_no_logging,  e_error,  e_warning,  e_info,  e_debug, e_macro };
   // log severity info.
   void info(const char* _message, ...);
   // log severity warning.
   void warning(const char* _message, ...);
   // log severity error.
   void error(const char* _message, ...);
   // log severity debug.
   void debug(const char* _message, ...);
   // logging specifically only for macros defined in log_macros.h
   void macro(const char* _message, ...);
   // log severity info.
   void info_inline(const char* _message, ...);
   // log severity warning.
   void warning_inline(const char* _message, ...);
   // log severity error.
   void error_inline(const char* _message, ...);
   // log severity debug.
   void debug_inline(const char* _message, ...);
   // logging specifically only for macros defined in log_macros.h
   void macro_inline(const char* _message, ...);
   // log with no topic.
   void no_topic(const char* _message, ...);
   // log hash::path
   void hash_path(hash::path&& _path);
   //
   class topic final : private params::callback
   {
     public:
      void logline(level _level, const char* _message, std::va_list);
      void log(level _level, const char* _message, std::va_list);
      const char* name() const { return m_name; }
      std::uint32_t hash() const { return m_hash; }
      level m_level = e_info;
     private:
      topic(const char* _topic, uint32_t _hash) : m_name(_topic), m_hash(_hash)
      {
	 m_cb_name = m_name;
      }
      bool param_cb(const char* _param_name, param_args _args) final;
      const char* m_name;
      std::uint32_t m_hash;
      friend class topics;
   };
   
   class topics
   {
     public:
      static void logline(level _level, const char* _message, std::va_list args);
      static void log(level _level, const char* _message, std::va_list args);
      template<typename T> static bool add(T&& _topic)
      {
	 auto hash = hash::i32(_topic);
	 return topics::add_topic_internal(new topic(_topic, hash));
      }
      template<typename T> static bool set(T&& _topic)
      {
	 return topics::set_topic_internal(hash::i32(_topic));
      }
      static const char* get();
      static uint32_t hash();
      template<typename T> static bool set_level(T&& _topic, log::level _level)
      {
	 return topics::set_level_internal(hash::i32(_topic), _level);
      }
      template<typename T> static log::level get_level(T&& _topic)
      {
	 return topics::get_level_internal(hash::i32(_topic));
      }
      static log::level get_level()
      {
	 return topics::get_level_internal();
      }
     private:
      friend class scope_topic;
      static bool add_topic_internal(topic* _topic);
      static bool set_topic_internal(std::uint32_t _hash);
      static bool set_level_internal(std::uint32_t _hash, log::level _level);
      static log::level get_level_internal(std::uint32_t _hash);
      static log::level get_level_internal();
      static std::map<std::uint32_t, std::unique_ptr<topic>> m_topics;
      static std::map<std::thread::id, std::uint32_t > m_thread_topic;
   };
   class scope_topic
   {
   public:
      scope_topic(uint32_t _hash)
      {
	 m_last_hash = topics::hash();
	 topics::set_topic_internal(_hash);
      }
      ~scope_topic()
      {
	 topics::set_topic_internal(m_last_hash);
      }
   private:
      uint32_t  m_last_hash;
   };
   template<typename T> static  scope_topic scope(T&& _topic)
   {
      auto hash = hash::i32(_topic);
      return scope_topic(hash);
   }
}
#endif//LOG_H
