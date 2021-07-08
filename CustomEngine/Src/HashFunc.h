#pragma once
#ifndef HASHFUNC_H
#define HASHFUNC_H

#include <unordered_map>
#include <string>

template <class T>
struct charPtrEqualTo : public std::binary_function<T, T, bool>
{
  bool operator()(const T& x, const T& y) const
  {
    return std::strcmp(x, y) == 0;
  }
};



#endif