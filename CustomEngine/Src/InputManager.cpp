#include "InputManager.h"

void InputManager::Update(GLFWwindow* window)
{
  for (unsigned int i = 0; i < keyboardFunctions.size(); i++)
  {
  }
}

int InputManager::RegisterKeyboard(void(*func)(int key))
{
  int ID = -1;
  if (kbFree.empty())
  {
    ID = keyboardFunctions.size();
    keyboardFunctions.push_back(func);
  }
  else
  {
    ID = kbFree.back();
    kbFree.pop_back();
    keyboardFunctions[ID] = func;
  }
  return ID;
}

int InputManager::RegisterMouse(void(*func)(double x, double y))
{
  int ID = -1;
  if (mFree.empty())
  {
    ID = mouseFunctions.size();
    mouseFunctions.push_back(func);
  }
  else
  {
    ID = mFree.back();
    mFree.pop_back();
    mouseFunctions[ID] = func;
  }
  return ID;
}