#pragma once
#ifndef CSTRING_H
#define CSTRING_H
#include <string>

class CString
{
public:
  static const char* Add(const char* a, const char* b);
  
  template<typename T>
  static const char* Add(const char* a, const T& val)
  {
    String.clear();
    String += a;
    String += std::to_string(val);
    return String.c_str();
  }

private:
  CString() {};
  ~CString() {};

  static std::string String;
};

#endif