#ifndef EDITOR_H
#define EDITOR_H
#include "Rect.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "Object.h"

typedef void(*DrawFunction)(Rect<unsigned int>);

struct MenuDrawer
{
  MenuDrawer() : funcPtr(nullptr), open(false) {};
  MenuDrawer(DrawFunction func) : funcPtr(func), open(false) {};

  DrawFunction funcPtr;
  bool open;
};

typedef void(*WindowFunction)();

class EditorMenu
{
public:
  typedef std::unordered_map<std::string, MenuDrawer> menuMap;

  EditorMenu(const std::string& name, const Rect<unsigned int>& window) : ID(0), Name(name), Open(true), Window(window), Menus(), MenuSelect(nullptr) {};
  ~EditorMenu() {};

  void Draw();
  void Add(const std::string& name, void (*drawdebug)(Rect<unsigned int>));
  void Select(const std::string& name);
  void Remove(const std::string& name);

  menuMap& GetMap() { return Menus; };
  const std::string& GetName() { return Name; };

private:
  unsigned int ID;
  std::string Name;
  Rect<unsigned int> Window;
  menuMap Menus;
  DrawFunction MenuSelect;
  bool Open;
};

class Editor
{
public:
  enum WindowSystems
  {
    WS_Time, WS_Window, WS_Renderer, WS_Profiler, WS_COUNT
  };

  static void Init();
  static void Exit();

  static void DrawMenus();
  static void AddMenu(EditorMenu &menu);
  static EditorMenu* GetMenu(const std::string &name);
  static void SelectMenu(const std::string& menu, const std::string &submenu);
  static void RemoveMenu(unsigned int id);

  static void DrawFileMenu();
  static void DrawToolbar();

  static void DrawObjEditor(Rect<unsigned int> window);
  static void SetObjEditor(DrawFunction ObjEditor);

  static void DrawConsoleMenu(Rect<unsigned int> window);
  static void WriteToConsole(const std::string& log);

private:
  Editor() {};
  ~Editor() {};

  static std::vector<EditorMenu> Menus;

  static WindowFunction Windows[WS_COUNT];
  static bool WindowsOpen[WS_COUNT];

  static DrawFunction ObjEditor;
};

#endif