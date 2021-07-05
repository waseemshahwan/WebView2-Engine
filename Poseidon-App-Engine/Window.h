#pragma once
#include <Windows.h>
#include <future>
#include <map>

class Window
{

	std::thread windowThread;
	bool isActive = true;

public:

	Window();
	~Window();

	HWND hWindow;
	std::future<void> messageHandler;

	void RunAsync(std::function<void()> callback);
	bool ProcessEvents();
	HWND StartWindow(bool headless);
	bool StopWindow();
	static LRESULT CALLBACK WindowProcedure(HWND window, unsigned int msg, WPARAM wp, LPARAM lp);

};

