#ifndef LOG_H
#define LOG_H
#include <map>
#include <memory>
#include <thread>
#include <iosfwd>
/* class std::string; */

namespace log
{
   enum level {  e_none,  e_error,  e_warning,  e_info,  e_debug, e_nologging };
   // adds a topic to the global list.
   bool add_topic(const char* _topic);
   // sets the topic, for the current thread.
   bool set_topic(const char* _topic);
   // gets the topic, for the current thread.
   const char* get_topic();
   // sets the log level of a topic.
   bool set_level(const char* _topic, level _level);
   // log severity info.
   void info(const char* _message, ...);
   // log severity warning.
   void warning(const char* _message, ...);
   // log severity error.
   void error(const char* _message, ...);
   // log severity debug.
   void debug(const char* _message, ...);
   // log with no topic.
   void no_topic(const char* _message, ...);

   class scope_topic
   {
   public:
      scope_topic(const char* _topic)
      {
	 m_last_topic = log::get_topic();
	 log::set_topic(_topic);
      }
   private:
      ~scope_topic()
      {
	 log::set_topic(m_last_topic);
      }
      const char* m_last_topic;
   };
   class topic
   {
     public:
      void log(level _level, const char* _message, ...);
      const char* literal() const { return m_topic.c_str(); }
      level m_level = e_info;
     private:
     topic(const std::string& _topic) : m_topic(_topic){}
     topic(const std::string& _topic, level _level) : m_level(_level), m_topic(_topic){}
      const std::string m_topic;
      friend bool log::add_topic(const char* _topic);
      friend class topics;
   };
   class topics
   {
     public:
      static void log(level _level, const char* _message, ...);
     private:
      friend bool log::add_topic(const char* _topic);
      friend bool log::set_topic(const char* _topic);
      friend const char* get_topic();
      friend bool log::set_level(const char* _topic, level _level);
      static std::map<const char*, std::unique_ptr<topic>> m_topics;
      static std::map<std::thread::id, const char*> m_thread_topic;
   };
}
#endif//LOG_H
