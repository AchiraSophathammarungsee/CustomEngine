#include "CString.h"

std::string CString::String;

const char* CString::Add(const char* a, const char* b)
{
  String.clear();
  String += a;
  String += b;
  return String.c_str();
}