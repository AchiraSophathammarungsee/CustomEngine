#pragma once
#ifndef SPARSESET_H
#define SPARSESET_H

#include "EngineCore.h"
#include <vector>

template<typename T>
class SparseSet
{
public:
  SparseSet();
  SparseSet(size_t size, const T& initVal);

  void Add(const T& value)
  {
    ASSERT(Contains(value) == false, "[Sparse Set] Tried to add duplicated value: {}", value);
    Sparse.push_back(Packed.size());
    Packed.push_back(value);
  }

  void Get(size_t id)
  {

  }

  void Remove()
  {

  }

  void Clear()
  {
    Packed.clear();
    Sparse.clear();
  }

  void Reserve(size_t size);

  size_t Size() const { Packed.size(); };
  bool Empty() const { Packed.empty(); };
  bool Contains(const T& value) const
  {
    return Sparse[value];
  }
  size_t Index(const T &value) const;

private:
  std::vector<T> Packed;
  std::vector<size_t> Sparse;
};

#endif