//
//#ifndef INPUT_H
//#define INPUT_H
//
//#include <GLFW/glfw3.h>
//#include "Bitmask.h"
//#include <cstdint>
//#include <vector>
//#include "KeyEvent.h"
//#include "MouseEvent.h"
//#include "KeyCode.h"
//
//class ActionMap
//{
//public:
//  typedef uint32_t ActionID;
//  typedef uint32_t ButtonID;
//
//  ActionMap();
//  ~ActionMap();
//
//  void Update();
//  void Clear();
//  void CaptureInput(Key key);
//  void CaptureInput(Mouse key);
//
//  bool IsKeyPressed(ActionID action);
//  bool IsKeyDown(ActionID action);
//  bool IsKeyReleased(ActionID action);
//
//private:
//  uint32_t MaxActions;
//
//  //std::vector<ActionID> KeyboardMap[KEY_COUNT];
// // std::vector<ActionID> MouseMap[MOUSE_COUNT];
//
//  Bitmask CurrFrameActions;
//  Bitmask PrevFrameActions;
//};
//
//class Input
//{
//public:
//  using EventCallbackFn = std::function<void(Event&)>;
//
//  enum InputModes { IM_EDITOR, IM_GAME, IM_COUNT };
//
//  static void Init();
//  static void Update();
//  static void Exit();
//
//  static void SetEventCallback(const EventCallbackFn& callback);
//
//  static void SetInputMode(InputModes mode);
//  static InputModes GetInputMode() { return InputMode; };
//
//  static bool IsKeyPressed(Key key);
//  static bool IsKeyPressed(Mouse key);
//  static bool IsKeyReleased(Key key);
//  static bool IsKeyReleased(Mouse key);
//
//  static void StartCaptureTextInput(char *textBuffer, size_t bufferSize);
//  static void EndCaptureTextInput();
//
//private:
//  Input() {};
//  ~Input() {};
//
//  static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mod);
//  static void MouseCallback(GLFWwindow* window, int button, int action, int mod);
//  static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
//  static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
//
//  static InputModes InputMode;
//  static ActionMap ActionMaps[IM_COUNT];
//};
//
//#endif