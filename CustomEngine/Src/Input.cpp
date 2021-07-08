//#include "Input.h"
//#include "Window.h"
//
//void Input::Init()
//{
//  glfwSetKeyCallback(Window::GetWindowHandler(), KeyboardCallback);
//  glfwSetMouseButtonCallback(Window::GetWindowHandler(), MouseCallback);
//  glfwSetScrollCallback(Window::GetWindowHandler(), MouseScrollCallback);
//}
//
//void Input::Update()
//{
//  ActionMaps[InputMode].Update();
//}
//
//void Input::Exit()
//{
//
//}
//
//void Input::SetInputMode(InputModes mode)
//{
//  InputMode = mode;
//  for (size_t i = 0; i < IM_COUNT; i++)
//  {
//    ActionMaps[i].Clear();
//  }
//}
//
//void Input::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
//{
//  switch (action)
//  {
//    case(GLFW_PRESS):
//    {
//      KeyPressedEvent keyEvent(key, 0);
//      break;
//    }
//    case(GLFW_REPEAT):
//    {
//      KeyPressedEvent keyEvent(key, 1);
//      break;
//    }
//    case(GLFW_RELEASE):
//    {
//      KeyReleasedEvent keyEvent(key);
//      break;
//    }
//  }
//
//  if(action == GLFW_PRESS)
//    ActionMaps[InputMode].CaptureInput(static_cast<Key>(key));
//}
//
//void Input::MouseCallback(GLFWwindow* window, int button, int action, int mod)
//{
//  switch (action)
//  {
//    case(GLFW_PRESS):
//    {
//      MouseButtonPressedEvent mouseEvent(button);
//      EventCallback(mouseEvent);
//      break;
//    }
//    case(GLFW_RELEASE):
//    {
//      MouseButtonReleasedEvent mouseEvent(button);
//      break;
//    }
//  }
//
//  if (action == GLFW_PRESS)
//    ActionMaps[InputMode].CaptureInput(static_cast<Mouse>(button));
//}
//
//void Input::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
//{
//  MouseScrollEvent mouseEvent(xoffset, yoffset);
//  EventCallback(mouseEvent);
//}
//
//void Input::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
//{
//  MouseMovedEvent mouseEvent(xpos, ypos);
//  EventCallback(mouseEvent);
//}
//
//bool Input::IsKeyPressed(Key key)
//{
//  return glfwGetKey(Window::GetWindowHandler(), key) == GLFW_PRESS;
//}
//
//bool Input::IsKeyPressed(Mouse key)
//{
//  return glfwGetMouseButton(Window::GetWindowHandler(), key) == GLFW_PRESS;
//}
//
//bool Input::IsKeyReleased(Key key)
//{
//  return glfwGetKey(Window::GetWindowHandler(), key) == GLFW_RELEASE;
//}
//
//bool Input::IsKeyReleased(Mouse key)
//{
//  return glfwGetMouseButton(Window::GetWindowHandler(), key) == GLFW_RELEASE;
//}
//
//void ActionMap::Update()
//{
//  PrevFrameActions = CurrFrameActions;
//  CurrFrameActions.Clear();
//}
//
//void ActionMap::Clear()
//{
//  PrevFrameActions.Clear();
//  CurrFrameActions.Clear();
//}
//
//void ActionMap::CaptureInput(InputKey::Key key)
//{
//  for (ActionID action : KeyboardMap[key])
//  {
//    CurrFrameActions.SetBit(action, true);
//  }
//}
//
//void ActionMap::CaptureInput(InputKey::MouseKey key)
//{
//  for (ActionID action : MouseMap[key])
//  {
//    CurrFrameActions.SetBit(action, true);
//  }
//}
//
//bool ActionMap::IsKeyPressed(ActionID action)
//{
//  return CurrFrameActions.GetBit(action) && !PrevFrameActions.GetBit(action);
//}
//
//bool ActionMap::IsKeyDown(ActionID action)
//{
//  return CurrFrameActions.GetBit(action);
//}
//
//bool ActionMap::IsKeyReleased(ActionID action)
//{
//  return !CurrFrameActions.GetBit(action) && PrevFrameActions.GetBit(action);
//}