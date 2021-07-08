#include "Log.h"

std::ofstream Log::fileptr;
std::ostringstream Log::logger;
std::string Log::ErrorMessages[Count] = { "[Trace]: ", "[Warning]: ", "[ERROR]: ", "[FATAL ERROR]: " };
Log::LogOutput Log::output;

bool Log::Init(void)
{
  output = Console;

  fileptr.open("Log.txt", std::ios::trunc);
  if (!fileptr)
  {
    output = Console;
    LOG_ERROR("Log", "couldn't open log file");
    return false;
  }
  return true;
}

void Log::Write(const char* systemtype, LogPriorities priority, const char* log)
{
  Platform::SetConsoleTextColor(static_cast<Platform::TextColor>(priority));

  std::string date = Time::GetDate();

  if (output == Log::Console)
  {
    std::cout << date << "| " << '[' << systemtype << ']' << ErrorMessages[priority] << log << std::endl;
  }
  else if (output == Log::File)
  {
    if (fileptr)
    {
      if (fileptr.bad())
        std::cout << "ERROR: Log couldn't write to file" << std::endl;
      else
        fileptr << date << "| " << '[' << systemtype << ']' << ErrorMessages[priority] << log << std::endl;
    }
  }
  else
  {
    if (fileptr)
    {
      if (fileptr.bad())
        std::cout << "ERROR: Log couldn't write to file" << std::endl;
      else
        fileptr << date << "| " << '[' << systemtype << ']' << ErrorMessages[priority] << log << std::endl;
    }
    
    std::cout << date << "| " << '[' << systemtype << ']' << ErrorMessages[priority] << log << std::endl;
  }
  
  Platform::SetConsoleTextColor(static_cast<Platform::TextColor>(0));
}

void Log::End()
{
  if (output == Log::Console)
  {
    std::cout << logger.str() << std::endl;
  }
  else if (output == Log::File)
  {
    if (fileptr)
    {
      if (fileptr.bad())
        std::cout << "ERROR: Log couldn't write to file" << std::endl;
      else
        fileptr << logger.str() << std::endl;
    }
  }
  else
  {
    if (fileptr)
    {
      if (fileptr.bad())
        std::cout << "ERROR: Log couldn't write to file" << std::endl;
      else
        fileptr << logger.str() << std::endl;
    }

    std::cout << logger.str() << std::endl;
  }

  logger.str("");
  logger.clear();

  Platform::SetConsoleTextColor(static_cast<Platform::TextColor>(0));
}

void Log::Clear(void)
{
  if (fileptr)
    fileptr.close();
}
