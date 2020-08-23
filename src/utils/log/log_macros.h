#ifndef LOG_MACROS_H
#define LOG_MACROS_H

// Log macros are intended for work in progress code and shouldn't be commited.
// May add a specific git hook to make sure it's not included in any file.

#undef log_int
#undef log_uint
#undef log_float
#undef log_str
#undef log_message

#define log_int(variable) log::macro("%s:%d: ["#variable"]: %d", __FILE__, __LINE__, variable);
#define log_uint(variable) log::macro("%s:%d: ["#variable"]: %u", __FILE__, __LINE__, variable);
#define log_float(variable) log::macro("%s:%d: ["#variable"]: %f", __FILE__, __LINE__, variable);
#define log_str(variable) log::macro("%s:%d: ["#variable"]: %s", __FILE__, __LINE__, variable);
#define log_message(message) log::macro("%s:%d: %s", __FILE__, __LINE__, message);


#endif//LOG_MACROS_H
