
#ifndef ENGINECORE_H
#define ENGINECORE_H

#include <memory>
#include "Log.h"
#include <imgui.h>
#include <memory>

#ifdef _WIN32
  #ifdef _WIN64
    #define PLATFORM_WINDOWS
    #error "Windows x64 is not suported!"
  #else
    #define PLATFORM_WINDOWS
  #endif
#elif defined(__APPLE__) || defined(__MACH__)
  #include <TargetConditionals.h>
  /* TARGET_OS_MAC exists on all the platforms
     * so we must check all of them (in this order)
     * to ensure that we're running on MAC
     * and not some other Apple platform */
  #if TARGET_IPHONE_SIMULATOR == 1
    #error "IOS simulator is not supported!"
  #elif TARGET_OS_IPHONE == 1
    #define HZ_PLATFORM_IOS
    #error "IOS is not supported!"
  #elif TARGET_OS_MAC == 1
    #define HZ_PLATFORM_MACOS
    #error "MacOS is not supported!"
  #else
    #error "Unknown Apple platform!"
  #endif
#elif defined(__ANDRIOD__)
  #define PLATFORM_ANDRIOD
  #error "Andriod is not supported!"
#elif defined(__linux__)
  #define PLATFORM_LINUX
  #error "Linux is not supported!"
#else
  #error "Unknown platform!"
#endif

#define EDITOR_MODE

#ifdef DEBUG
  #if defined(PLATFORM_WINDOWS)
    #define DEBUGBREAK() __debugbreak()
  #elif defined(PLATFORM_LINUX)
    #include <signal.h>
    #define DEBUGBREAK() raise(SIGTRAP)
  #else
    #error "Platform doesn't support debugbreak yet!"
  #endif
  #define ENABLE_ASSERTS
#else
  #define DEBUGBREAK()
#endif

#ifdef ENABLE_ASSERTS
  #define ASSERT(x, msg, ...) { if(!(x)) { LOG_ERROR_S("Assert", msg, __VA_ARGS__); DEBUGBREAK(); } }
#else
  #define ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Unique = std::unique_ptr<T>;

#endif