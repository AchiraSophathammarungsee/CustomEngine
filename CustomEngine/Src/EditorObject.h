
#ifndef EDITOROBJECT_H
#define EDITOROBJECT_H

#include <string>

template<size_t MAXOBJ>
class EditObjects
{
public:
  const std::string& GetName(size_t objID) { return Names[objID]; };
  void SetName(size_t objID, const char* name) { Names[objID] = name; };

private:
  std::string Names[MAXOBJ];
};

#endif