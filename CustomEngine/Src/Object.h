#ifndef OBJECT_H
#define OBJECT_H

#include <string>

class Object
{
public:
  Object(const std::string &name = "Unnamed Object");
  ~Object() {};

  const std::string& GetName() { return Name; };
  void SetName(const std::string& newName) { Name = newName; strcpy_s(nameBuffer, Name.c_str()); };

  virtual void DrawDebugMenu();

private:
  std::string Name;
  char nameBuffer[32];
};

#endif