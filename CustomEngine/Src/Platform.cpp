#include "Platform.h"

#ifdef _WIN32
  #include <windows.h>
#endif

namespace Platform
{
#ifdef _WIN32
  static const int winTextColors[] = { 15, 14, 12, 13, 10, 9, 0 };

  void SetConsoleTextColor(TextColor color)
  {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), winTextColors[color]);
  }

#else
  static const char *osTextColors[] = { "\033[37m", "\033[33m", "\033[31m", "\033[35m", "\033[32m", "\033[34m", "\033[30m" };

  void SetConsoleTextColor(TextColor color)
  {
    std::cout << osTextColors[color];
  }

#endif
}