#ifndef WINDOW_H
#define WINDOW_H
#include <GLFW/glfw3.h>
#include <string>

enum WindowModes
{
  WM_Windowed, WM_Fullscreen, WM_Borderless, WM_COUNT
};

class Window
{
public:
  static int Init(WindowModes mode = WM_Fullscreen, unsigned AAsample = 1);
  static void Exit();
  
  static void MakeCurrentContext();
  static void SwapBuffer();
  static void PoleEvents();
  static int ShouldClose();

  static unsigned Width() { return width; };
  static unsigned Height() { return height; };

  static void GetCursorPos(unsigned int& x, unsigned int& y);
  static void GetCursorPos(double& x, double& y);
  static int GetKey(int key);

  static GLFWwindow* GetWindowHandler() { return window; };
  static void SetKeyCallback(GLFWkeyfun function);
  static void SetMouseButtonCallback(GLFWmousebuttonfun function);
  static void SetCursorPosCallback(GLFWcursorposfun function);
  static void SetScrollCallback(GLFWscrollfun function);

private:
  Window() {};
  ~Window() {};

  static GLFWwindow* window;

  static unsigned width;
  static unsigned height;

  static unsigned AAsample;

  static WindowModes windowMode;
};

#endif