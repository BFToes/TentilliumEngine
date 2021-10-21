#pragma once

class AppWindow
{
private:
	int m_Width, m_Height;
	const char* m_Title;
	bool m_Vsync;
	void* m_Window;

public:
	AppWindow(int width, int height, const char* title, bool Vsync = true);
	~AppWindow();
	void MakeCurrent();
	void Refresh();
	bool Closed() const;
};

