#pragma once
#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <string>

namespace FileDialog
{
  size_t GetMaxFileBuffer();

  std::string CurrPath();

  // return empty string if failed
  std::string OpenFile(const char *filter);

  // return empty string if failed
  std::string SaveFile(const char *filter);
}

#endif