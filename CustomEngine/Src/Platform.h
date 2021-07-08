
#ifndef PLATFORM_H
#define PLATFORM_H

namespace Platform
{
  enum TextColor { White, Yellow, Red, Purple, Green, Blue, Black };
  void SetConsoleTextColor(TextColor color);

  void CreateThread(unsigned int id);
}

#endif