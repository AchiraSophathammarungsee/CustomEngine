
#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include "Window.h"
#include "Event.h"
#include "WindowEvent.h"
#include "Scene.h"

int main(int argc, char** argv);

class Application
{
public:
  Application(const std::string& name = "Game Engine");
  virtual ~Application() {};

  void OnEvent(Event &e);
  void Close();

  static Application& Get() { return *StaticInstance; };

private:
  void Run();
  bool OnWindowClose(WindowCloseEvent &e);
  bool OnWindowResize(WindowResizeEvent &e);

  bool Running = true;
  bool Minimized = false;
  Scene* scene;

  static Application* StaticInstance;
  friend int ::main(int argc, char** argv);
};

// To be defined in CLIENT
Application* CreateApplication();

#endif