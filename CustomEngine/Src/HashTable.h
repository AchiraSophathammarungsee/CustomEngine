#pragma once
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include "EngineCore.h"

template<typename T>
class HashTable
{
public:
  HashTable()
  {
    for (int i = 0; i < maxSize; i++)
    {
      keys[i] = std::string();
      values[i] = T();
    }
  }

  T Find(const std::string& key) const
  {
    int index = FindIndex(key);
    if (index != -1)
    {
      return values[index];
    }

    LOG_ERROR_S("Hash table", "Can't find index from {}", key);

    return T();
  }

  void Insert(const std::string& key, const T& val)
  {
    int index = FindIndex(key);
    if (index == -1) 
    {
      LOG_ERROR_S("Hash table", "Table is full while trying to insert {}", key);
      return;
    }

    keys[index] = key;
    values[index] = val;
  }

  bool Remove(const std::string& key)
  {
    int index = FindIndex(key);
    if (index == -1) return false;

    keys[index].clear();
    values[index].clear();

    return true;
  }

  void GetTable(std::string** outKeys, T** outValues, size_t *outMaxSize)
  {
    *outKeys = keys;
    *outValues = values;
    *outMaxSize = maxSize;
  }

  /*std::string PrintString()
  {
    std::string result;
    result += "{";

    for (int i = 0; i < size_max; i++)
      if (!keys[i].empty())
        result += keys[i] + ":" + std::to_string(values[i]) + ", ";

    result += "}";
  }*/

private:
  unsigned int hash(const std::string& key)
  {
    unsigned int value = 0;
    for (int i = 0; i < key.length(); i++)
    {
      value = value * 37 + key[i];
    }
    return value;
  }

  int FindIndex(const std::string& key, bool override_duplicate = true)
  {
    unsigned int h = hash(key) % maxSize, offset = 0, index;

    while (offset < maxSize)
    {
      index = (h + offset) % maxSize;

      // empty index for new entry with key `key`
      // or find the index of key `key` in hash table
      // if `override_duplicate_key` is `false`, return a new, unused index, used in DictionaryADT
      if (keys[index].empty() || (override_duplicate && keys[index] == key))
        return index;

      offset++;
    }
    return -1;
  }

  const static size_t maxSize = 200;

  std::string keys[maxSize];
  T values[maxSize];
};

#endif