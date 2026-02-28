#pragma once

class InputClass
{
public:
	InputClass();
	InputClass(const InputClass& inputClass);
	~InputClass();

	void Initialize();
	void BeginFrame();            // Call once per frame to reset per-frame deltas

	// Keyboard
	void KeyDown(unsigned int input);
	void KeyUp(unsigned int input);
	bool IsKeyDown(unsigned int key);

	// Mouse events (called from WndProc)
	void MouseMove(int x, int y);
	void MouseLeftDown(int x, int y);
	void MouseLeftUp(int x, int y);
	void MouseRightDown(int x, int y);
	void MouseRightUp(int x, int y);
	void MouseWheel(int delta);

	// Mouse state queries
	bool IsMouseLeftDown()    const;
	bool IsMouseRightDown()   const;
	int  GetMouseDeltaX()     const;
	int  GetMouseDeltaY()     const;
	int  GetMouseWheelDelta() const;

private:
	bool m_Keys[256];

	int  m_mouseX;
	int  m_mouseY;
	int  m_mouseDeltaX;
	int  m_mouseDeltaY;
	int  m_mouseWheelDelta;
	bool m_mouseLeftDown;
	bool m_mouseRightDown;
	bool m_firstMouseMove;    // suppress the initial large delta on first move
};