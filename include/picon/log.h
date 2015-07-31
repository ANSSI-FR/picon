#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>



#define _LOG_STR_ERROR "\033[31;01m[error]\033[00m"
#define _LOG_STR_DEBUG "\033[34;01m[debug]\033[00m"
#define _LOG_STR_TRACE "\033[33;01m[trace]\033[00m"

#define _LOG_STR_MONITOR "\033[35;01m[monitor]\033[00m"
#define _LOG_STR_CLIENT "\033[36;01m[ client]\033[00m"



#ifdef SILENT
#define _LOG(...)
#else
#define _LOG(...)                               \
  do {                                          \
    fprintf(stderr, __VA_ARGS__);               \
  } while(0)
#endif


#define _LOG_ERROR(...)                         \
  do {                                          \
    _LOG(_LOG_STR_ERROR "\t" __VA_ARGS__);      \
  } while(0)



#define LOG_ERROR_CLIENT(...)                           \
  do {                                                  \
    _LOG_ERROR(_LOG_STR_CLIENT "\t" __VA_ARGS__);       \
  } while(0)

#define LOG_ERROR_MONITOR(...)                          \
  do {                                                  \
    _LOG_ERROR(_LOG_STR_MONITOR "\t" __VA_ARGS__);      \
  } while(0)





#if defined CFI_DEBUG
#define _LOG_DEBUG(...)                         \
  do {                                          \
    _LOG(_LOG_STR_DEBUG "\t" __VA_ARGS__);      \
  } while(0)
#else
#define _LOG_DEBUG(...)
#endif





#define LOG_DEBUG_CLIENT(...)                           \
  do {                                                  \
    _LOG_DEBUG(_LOG_STR_CLIENT "\t" __VA_ARGS__);       \
  } while(0)

#define LOG_DEBUG_MONITOR(...)                          \
  do {                                                  \
    _LOG_DEBUG(_LOG_STR_MONITOR "\t" __VA_ARGS__);      \
  } while(0)



#define _LOG_TRACE(...)                         \
  do {                                          \
    _LOG(_LOG_STR_TRACE "\t" __VA_ARGS__);      \
  } while(0)
#else

#define LOG_TRACE_MONITOR(...)                          \
  do {                                                  \
    _LOG_TRACE(_LOG_STR_MONITOR "\t" __VA_ARGS__);      \
  } while(0)

#endif
