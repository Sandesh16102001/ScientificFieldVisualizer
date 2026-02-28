#include "InputClass.h"

InputClass::InputClass()
{
}

InputClass::InputClass(const InputClass& inputClass)
{
}

InputClass::~InputClass()
{
}

void InputClass::Initialize()
{
	for (int i = 0; i < 256; i++)
		m_Keys[i] = false;

	m_mouseX          = 0;
	m_mouseY          = 0;
	m_mouseDeltaX     = 0;
	m_mouseDeltaY     = 0;
	m_mouseWheelDelta = 0;
	m_mouseLeftDown   = false;
	m_mouseRightDown  = false;
	m_firstMouseMove  = true;
}

void InputClass::BeginFrame()
{
	// Reset per-frame deltas so they don't accumulate across frames
	m_mouseDeltaX     = 0;
	m_mouseDeltaY     = 0;
	m_mouseWheelDelta = 0;
}

void InputClass::KeyDown(unsigned int input)
{
	m_Keys[input] = true;
}

void InputClass::KeyUp(unsigned int input)
{
	m_Keys[input] = false;
}

bool InputClass::IsKeyDown(unsigned int key)
{
	return m_Keys[key];
}

void InputClass::MouseMove(int x, int y)
{
	if (m_firstMouseMove)
	{
		// Skip the first event to avoid a huge jump from (0,0) to cursor pos
		m_mouseX         = x;
		m_mouseY         = y;
		m_firstMouseMove = false;
		return;
	}

	m_mouseDeltaX += x - m_mouseX;
	m_mouseDeltaY += y - m_mouseY;
	m_mouseX       = x;
	m_mouseY       = y;
}

void InputClass::MouseLeftDown(int x, int y)
{
	m_mouseLeftDown = true;
	m_mouseX        = x;
	m_mouseY        = y;
}

void InputClass::MouseLeftUp(int x, int y)
{
	m_mouseLeftDown = false;
}

void InputClass::MouseRightDown(int x, int y)
{
	m_mouseRightDown = true;
	m_mouseX         = x;
	m_mouseY         = y;
}

void InputClass::MouseRightUp(int x, int y)
{
	m_mouseRightDown = false;
}

void InputClass::MouseWheel(int delta)
{
	m_mouseWheelDelta += delta;
}

bool InputClass::IsMouseLeftDown()    const { return m_mouseLeftDown; }
bool InputClass::IsMouseRightDown()   const { return m_mouseRightDown; }
int  InputClass::GetMouseDeltaX()     const { return m_mouseDeltaX; }
int  InputClass::GetMouseDeltaY()     const { return m_mouseDeltaY; }
int  InputClass::GetMouseWheelDelta() const { return m_mouseWheelDelta; }
