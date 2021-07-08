
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H

#include "Event.h"
#include <sstream>

class MouseMovedEvent : public Event
{
public:
  MouseMovedEvent(float x, float y) : MouseX(x), MouseY(y) {};

  inline float GetX() const { return MouseX; };
  inline float GetY() const { return MouseY; };

  std::string ToString() const override
  {
    std::stringstream ss;
    ss << "MouseMoveEvent: " << MouseX << ", " << MouseY;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
  float MouseX, MouseY;
};

class MouseScrollEvent : public Event
{
public:
  MouseScrollEvent(float xoffset, float yoffset) : XOffset(xoffset), YOffset(yoffset) {};

  inline float GetXOffset() const { return XOffset; };
  inline float GetYOffset() const { return YOffset; };

  EVENT_CLASS_TYPE(MouseScrolled)
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
  float XOffset, YOffset;
};

class MouseButtonEvent : public Event
{
public:
  inline int GetMouseButton() const { return KeyCode; };
  
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

protected:
  MouseButtonEvent(int keycode) : KeyCode(keycode) {};

  int KeyCode;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
  MouseButtonPressedEvent(int keycode) : MouseButtonEvent(keycode) {};

  std::string ToString() const override
  {
    std::stringstream ss;
    ss << "MouseButtonPressedEvent: " << KeyCode;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
  MouseButtonReleasedEvent(int keycode) : MouseButtonEvent(keycode) {};

  std::string ToString() const override
  {
    std::stringstream ss;
    ss << "MouseButtonReleasedEvent: " << KeyCode;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonReleased)
};

#endif