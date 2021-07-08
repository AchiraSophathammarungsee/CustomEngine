#include "Window.h"
#include <sstream>
#include <iomanip>
#include "Log.h"

#ifdef _WIN32
#include <windows.h>
#elif __unix__
#include <>
#endif

GLFWwindow* Window::window;

unsigned Window::width;
unsigned Window::height;
unsigned Window::AAsample;
WindowModes Window::windowMode;

static const char* winModeStr[WM_COUNT] = { "Windowed", "Fullscreen", "Borderless" };

int Window::Init(WindowModes mode, unsigned AAsample)
{
  windowMode = mode;
  LOG_TRACE_S("Window", "Initializing Window in {} mode", winModeStr[windowMode]);

  LOG_TRACE("Window", "Initializing GLFW...");
  // Initialise GLFW
  if (!glfwInit())
  {
    LOG_ERROR("Window", "Failed to initialize GLFW");
    return -1;
  }

  // Setting up OpenGL properties
  glfwWindowHint(GLFW_SAMPLES, AAsample); // change for anti-aliasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  if (windowMode == WM_Borderless)
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  LOG_TRACE_S("Window", "Hint: GLversion({},{}), Multiple sample size: {}", 4, 6, AAsample);

  LOG_TRACE("Window", "Creating Windows...");

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* monitorMode = glfwGetVideoMode(monitor);

  height = monitorMode->height;
  width = monitorMode->width;

  if (windowMode != WM_Fullscreen) monitor = nullptr;

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(width, height,  "Scene 1",  monitor,  nullptr);

  if (window == nullptr)
  {
    LOG_ERROR("Window", "Failed to open GLFW window. If you have an Intel GPU, they are not 4.0 compatible.");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  return 0;
}

void Window::Exit()
{
  LOG_TRACE("Window", "Terminating GLFW...");
  glfwTerminate();
}

void Window::MakeCurrentContext()
{
  glfwMakeContextCurrent(window);
}

void Window::SwapBuffer()
{
  glfwSwapBuffers(window);
}

void Window::PoleEvents()
{
  glfwPollEvents();
}

int Window::ShouldClose()
{
  return glfwWindowShouldClose(window);
}

void Window::GetCursorPos(double& x, double& y)
{
  glfwGetCursorPos(window, &x, &y);
}

void Window::GetCursorPos(unsigned int& outx, unsigned int& outy)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  outx = static_cast<unsigned int>(x);
  outy = static_cast<unsigned int>(y);
}

int Window::GetKey(int key)
{
  return glfwGetKey(window, key);
}

void Window::SetKeyCallback(GLFWkeyfun function)
{
  glfwSetKeyCallback(window, function);
}

void Window::SetMouseButtonCallback(GLFWmousebuttonfun function)
{
  glfwSetMouseButtonCallback(window, function);
}

void Window::SetCursorPosCallback(GLFWcursorposfun function)
{
  glfwSetCursorPosCallback(window, function);
}

void Window::SetScrollCallback(GLFWscrollfun function)
{
  glfwSetScrollCallback(window, function);
}