//
// Created by pushpak on 6/1/18.
//

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class InputManager
{

public:
  InputManager() = default;
  InputManager( int windowWidth, int windowHeight );
  ~InputManager();

public:
    int Init();
    void CleanUp();
    
    void Update(GLFWwindow *window);

    int RegisterKeyboard(void(*func)(int key));
    int RegisterMouse(void(*func)(double x, double y));

    void UnregisterKeyboard(int id);
    void UnregisterMouse(int id);
private:
    // data members
  std::vector<void(*)(int)> keyboardFunctions;
  std::vector<int> kbFree;

  std::vector<void(*)(double, double)> mouseFunctions;
  std::vector<int> mFree;
};


#endif //INPUTMANAGER_H
