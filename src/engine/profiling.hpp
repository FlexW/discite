#pragma once

#include <optick.h>

#if defined(DC_ENABLE_PROFILING)

#define DC_PROFILE_START_CAPTURE(...) OPTICK_START_CAPTURE(__VA_ARGS__)
#define DC_PROFILE_STOP_CAPTURE(...)  OPTICK_STOP_CAPTURE(__VA_ARGS__)
#define DC_PROFILE_SAVE_CAPTURE(...)  OPTICK_SAVE_CAPTURE(__VA_ARGS__)
#define DC_PROFILE_SHUTDOWN()         OPTICK_SHUTDOWN()

#define DC_PROFILE_FRAME(...)          OPTICK_FRAME(__VA_ARGS__)
#define DC_PROFILE_SCOPE(...)          OPTICK_EVENT(__VA_ARGS__)
#define DC_PROFILE_TAG(NAME, ...)      OPTICK_TAG(NAME, __VA_ARGS__)
#define DC_PROFILE_SCOPE_DYNAMIC(NAME) OPTICK_EVENT_DYNAMIC(NAME)
#define DC_PROFILE_THREAD(...)         OPTICK_THREAD(__VA_ARGS__)

#else

#define DC_PROFILE_START_CAPTURE(...) void(0)
#define DC_PROFILE_STOP_CAPTURE(...)  void(0)
#define DC_PROFILE_SAVE_CAPTURE(...)  void(0)
#define DC_PROFILE_SHUTDOWN()         void(0)

#define DC_PROFILE_FRAME(...)          void(0)
#define DC_PROFILE_FUNC(...)           void(0)
#define DC_PROFILE_TAG(NAME, ...)      void(0)
#define DC_PROFILE_SCOPE_DYNAMIC(NAME) void(0)
#define DC_PROFILE_THREAD(...)         void(0)

#endif
