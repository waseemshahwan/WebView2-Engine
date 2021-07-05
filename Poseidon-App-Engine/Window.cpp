#include "Window.h"
#include <iostream>	
#include "Utility.h"

static constexpr UINT s_runAsyncWindowMessage = WM_APP;

Window::Window() {}
Window::~Window() {}

unsigned int windowCounter = 0;

HWND Window::StartWindow(bool headless) {

	//this->windowThread = std::thread([this] {

	std::string randomClassName = Utility::RandomString(16);
	std::wstring wClassName = Utility::AS2WS(randomClassName);

	const char* myclass = randomClassName.c_str();

	WNDCLASSEX wndclass = {
		sizeof(WNDCLASSEX), //cbSize
		CS_DBLCLKS, // Style
		Window::WindowProcedure, // Window procedure
		0, // ClsExtra
		0, // WndExtra
		GetModuleHandle(0), // hInstance
		LoadIcon(0,IDI_APPLICATION), // hIcon
		LoadCursor(0,IDC_ARROW), // hCursor
		HBRUSH(COLOR_WINDOW + 1), // hbrBackground
		0, // MenuName
		wClassName.c_str(), // ClassName
		LoadIcon(0, IDI_APPLICATION) // hIconSm
	};
	HWND hWnd;

	if (RegisterClassEx(&wndclass))
	{
		HWND window;
		if (headless) {
			window = CreateWindowExW(WS_EX_TOOLWINDOW, wClassName.c_str(), L"Poseidon", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
				CW_USEDEFAULT, 0, 0, 0, 0, 0, GetModuleHandle(0), nullptr);
		}
		else {

			int row = 0;
			int column = 0;
			int superficialCounter = 0;
			for (int x = 0; x < 8; x++) {
				for (int y = 0; y < 3; y++) {
					if (windowCounter == superficialCounter) {
						column = x;
						row = y;
					}
					superficialCounter++;
				}
			}

			windowCounter++;

			window = CreateWindowEx(
				0,  // Style
				wClassName.c_str(), // Class name
				L"Poseidon", // Window name
				WS_OVERLAPPEDWINDOW,  // Style
				column * 600, // X
				row * 600, // Y
				600, // Width
				600, // Height
				0, // hWND Parent
				0, // hMenu
				GetModuleHandle(0), // hInstance
				0 // lpParam
			);

		}

		this->hWindow = window;
	}

	std::cout << Utility::GetLastErrorAsString() << std::endl;

	if (!this->hWindow) {
		throw std::exception("Failed to launch win32 window");
	}

	if (this->hWindow)
	{
		SetWindowLongPtr(this->hWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
		ShowWindow(this->hWindow, SW_SHOWDEFAULT);
		UpdateWindow(this->hWindow);
	}

	//});

	return this->hWindow;
}

void Window::RunAsync(std::function<void()> callback) {
	auto* task = new std::function<void()>(std::move(callback));
	PostMessage(this->hWindow, s_runAsyncWindowMessage, reinterpret_cast<WPARAM>(task), 0);
}

bool Window::ProcessEvents()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) && this->isActive) {
		//std::cout << "Got message" << std::endl;
		DispatchMessage(&msg);
	}

	return true;
}

LRESULT CALLBACK Window::WindowProcedure(HWND window, unsigned int msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{

	case s_runAsyncWindowMessage:
	{

		auto* task = reinterpret_cast<std::function<void()>*>(wp);
		if (task != nullptr) {
			try {
				(*task)();
			}
			catch (std::exception err) {
				std::cout << "Error evaluting function: " << err.what() << std::endl;
			}
			delete task;
		}
		return true;
	}
	break;
	case WM_DESTROY:
		//std::cout << "\ndestroying window\n";
		PostQuitMessage(0);
		return 0;
	default:
		//std::cout << '.';
		return DefWindowProc(window, msg, wp, lp);
	}
}

bool Window::StopWindow() {
	this->isActive = false;
	CloseWindow(this->hWindow);
	return true;
}