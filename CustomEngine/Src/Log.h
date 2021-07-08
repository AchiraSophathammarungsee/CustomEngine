#ifndef LOG_H
#define LOG_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Time.h"
#include "Platform.h"

class Log
{
  public:

    enum LogPriorities { Normal = 0, Warning = 1, Error = 2, Fatal = 3, Count };
    enum LogOutput { Console, File, Both };

    static bool Init(void);
    static void SetOutputMode(LogOutput out) { output = out; };
    static void Write(const char* systemtype, LogPriorities priority, const char* log);

    template<typename... Args>
    static void Write(const char* systemtype, LogPriorities priority, const char* fmt, Args const& ... args)
    {
      Platform::SetConsoleTextColor(static_cast<Platform::TextColor>(priority));
      logger << Time::GetDate() << "| " << '[' << systemtype << ']' << ErrorMessages[priority];

      std::stringstream argss;
      std::vector<std::string> argsString;
      GetArgs(argsString, argss, (args)...);
      size_t argIndex = 0;

      while (*fmt != '\0')
      {
        // replace {} with next variable
        if (*fmt == '{' && *(fmt + 1) == '}' && (argIndex < argsString.size()))
        {
          logger << argsString[argIndex];
          argIndex++;
          fmt++;
        }
        else
        {
          logger << *fmt;
        }

        fmt++;
      }

      End();
    }

    template<typename T, typename... Args>
    static void GetArgs(std::vector<std::string> &out, std::stringstream &ss, const T& first, Args const& ... args)
    {
      ss << first;
      out.push_back(ss.str());
      ss.str("");
      ss.clear();
      GetArgs(out, ss, (args)...);
    }

    template<typename T>
    static void GetArgs(std::vector<std::string>& out, std::stringstream& ss, const T& first)
    {
      ss << first;
      out.push_back(ss.str());
      ss.clear();
    }

    static void End();
    static void Clear(void);

  private:
    Log() {};
    ~Log() {};

    static LogOutput output;
    static std::string ErrorMessages[Count];
    static std::ostringstream logger;
    static std::ofstream fileptr;
};

#ifdef DEBUG

#define LOG_TRACE(system, msg) ::Log::Write(system, Log::Normal, msg)
#define LOG_WARN(system, msg) ::Log::Write(system, Log::Warning, msg)
#define LOG_ERROR(system, msg) ::Log::Write(system, Log::Error, msg)
#define LOG_FATAL(system, msg) ::Log::Write(system, Log::Fatal, msg)

#define LOG_TRACE_S(system, fmt, ...) ::Log::Write(system, Log::Normal, fmt, __VA_ARGS__)
#define LOG_WARN_S(system, fmt, ...) ::Log::Write(system, Log::Warning, fmt, __VA_ARGS__)
#define LOG_ERROR_S(system, fmt, ...) ::Log::Write(system, Log::Error, fmt, __VA_ARGS__)
#define LOG_FATAL_S(system, fmt, ...) ::Log::Write(system, Log::Fatal, fmt, __VA_ARGS__)

#else

#define LOG_TRACE(system, msg)
#define LOG_WARN(system, msg)
#define LOG_ERROR(system, msg)
#define LOG_FATAL(system, msg)

#define LOG_TRACE_S(system, fmt, ...)
#define LOG_WARN_S(system, fmt, ...)
#define LOG_ERROR_S(system, fmt, ...)
#define LOG_FATAL_S(system, fmt, ...)

#endif

#endif
