
#ifndef KEYEVENT_H
#define KEYEVENT_H

#include "Event.h"
#include <sstream>

class KeyEvent : public Event
{
public:
  inline int GetKeyCode() const {};

  EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
  KeyEvent(int keycode) : KeyCode(keycode) {};

  int KeyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
  KeyPressedEvent(int keycode, int repeatcount) : KeyEvent(keycode), RepeatCount(repeatcount) {};

  inline int GetRepeatCount() const { return RepeatCount; };

  std::string ToString() const override
  {
    std::stringstream ss;
    ss << "KeyPressedEvent: " << KeyCode << " (" << RepeatCount << " repeats)";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyPressed)
private:
  int RepeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
  KeyReleasedEvent(int keycode) : KeyEvent(keycode) {};

  std::string ToString() const override
  {
    std::stringstream ss;
    ss << "KeyReleaseEvent: " << KeyCode;
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyReleased)
};

#endif