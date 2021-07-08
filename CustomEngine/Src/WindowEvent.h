
#ifndef WINDOWEVENT_H
#define WINDOWEVENT_H

#include "Event.h"
#include <sstream>

class WindowCloseEvent : public Event
{
public:
  WindowCloseEvent() {};

  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowResizeEvent : public Event
{
public:
  WindowResizeEvent(unsigned int width, unsigned int height) : Width(width), Height(height) {};

  std::string ToString() const override
  {
    std::stringstream ss;
    ss << "WindowResizeEvent: " << Width << ", " << Height;
    return ss.str();
  }

  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
private:
  unsigned int Width, Height;
};

class WindowFocusEvent : public Event
{
public:
  WindowFocusEvent() {};

  EVENT_CLASS_TYPE(WindowFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowLostFocusEvent : public Event
{
public:
  WindowLostFocusEvent() {};

  EVENT_CLASS_TYPE(WindowLostFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowMovedEvent : public Event
{
public:
  WindowMovedEvent() {};

  EVENT_CLASS_TYPE(WindowMoved)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

#endif