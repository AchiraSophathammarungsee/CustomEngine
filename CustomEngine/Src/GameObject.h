#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <string>

class GameObject
{
public:
  GameObject();
  ~GameObject();



private:
  unsigned id;
  std::string name;
  bool isDestroy;
};

#endif