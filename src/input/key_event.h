#pragma once
#if 0
#include "input/event.h"
#include "input/key_codes.h"
#include <sstream>

namespace medicimage
{

class KeyEvent : public Event
{
public:
	KeyCode GetKeyCode() const { return m_keyCode; }

	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
	KeyEvent(const KeyCode keycode) : m_keyCode(keycode) {}

	KeyCode m_keyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
	KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
		: KeyEvent(keycode), m_IsRepeat(isRepeat) {}

	bool IsRepeat() const { return m_IsRepeat; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << m_keyCode << " (repeat = " << m_IsRepeat << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed)
private:
	bool m_IsRepeat;
};

class KeyTextInputEvent : public Event
{
public:
  KeyTextInputEvent(const std::string& inputText) : m_inputText(inputText) {}
  EVENT_CLASS_CATEGORY(EventCategoryInput)
  EVENT_CLASS_TYPE(KeyTextInput)

  const std::string& GetInputTextText() const { return m_inputText; }

  std::string ToString() const override 
  {
    std::stringstream ss;
    ss << "KeyTextInputEvent: text:" << m_inputText;
    return ss.str(); 
  }
private:
  std::string m_inputText;
};

} // namespace medicimage
#endif