#include <thread>
#include <iostream>
#include "Gate.h"
#include "Utility.h"
#include <wrl.h>
#include <wil/result.h>
#include <wil/com.h>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include <vector>
#include <exception>

using namespace std;

Gate::Gate() {
	this->_id = (string)Utility::RandomString(16);
}

Gate::~Gate() {}

void Gate::addEventHandlers() {
	// On navigation completed ... do nothing? idk
	EventRegistrationToken frameNavigationCompletedToken;
	this->window->RunAsync([this, &frameNavigationCompletedToken] {
		this->webview.webview->add_FrameNavigationCompleted(
			Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
				{
					std::cout << "Navigation completed" << std::endl;
					return S_OK;
				}
			).Get(),
			&frameNavigationCompletedToken
		);
	});

	this->eventRemovers.push_back([this, &frameNavigationCompletedToken] {
		this->webview.webview->remove_FrameNavigationCompleted(frameNavigationCompletedToken);
	});

	std::cout << "Adding event handler" << std::endl;
	EventRegistrationToken webMessageReceivedToken;
	this->window->RunAsync([this, &webMessageReceivedToken] {
		this->webview.webview->add_WebMessageReceived(
			Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
				[this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
				{

					LPWSTR p;
					args->TryGetWebMessageAsString(&p);
					std::string strEvent = Utility::LP2A(p);

					cout << "Got event from JS: " << strEvent << endl;

					//std::string(strMessage.begin(), strMessage.end());
					return S_OK;
				}
			).Get(),
			&webMessageReceivedToken
		);
	});

	this->eventRemovers.push_back([this, &webMessageReceivedToken] {
		this->webview.webview->remove_WebMessageReceived(webMessageReceivedToken);
	});

}

void Gate::removeEventHandlers() {
	for (int i = 0; i < this->eventRemovers.size(); i++) {
		std::function<void()> remover = this->eventRemovers[i];
		remover();
	}

	this->eventRemovers.clear();
}

bool Gate::initialize(bool headless, string proxy) {

	// Create Win32 window to host the webview browser
	this->window = new Window();
	Window* window = this->window;

	bool done = false;
	this->windowThread = std::thread([this, &headless, &done] {
		//windowThreadId = GetCurrentThreadId();
		this->window->StartWindow(headless);
		done = true;
		this->window->ProcessEvents();
	});
	while (!done) this_thread::sleep_for(0.1s);

	// Crate the webview2 environment and options
	wil::com_ptr<ICoreWebView2Environment> environment;
	done = false;
	this->window->RunAsync([&proxy, &window, &environment, &done]
	{
		auto opt = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
		vector<string> options = {
			"--disable-features=IsolateOrigins,site-per-process",
			"--disable-site-isolation-trials",
			"--disable-web-security",
			"--allow-running-insecure-content",
			"--enable-low-end-device-mode",
			"--disable-dev-shm-usage",
			"--no-sandbox"
		};

		if (!proxy.empty()) {
			options.push_back("--proxy-server=" + proxy);
		};

		string arguments = "";
		for (string option : options) {
			arguments += " " + option;
		};
			
		opt->put_AdditionalBrowserArguments(Utility::AS2WS(arguments).c_str());
		std::wstring userDataDirectory = Utility::GetAppDataDirectory() + L"\\PoseidonProfile_" + Utility::AS2WS(Utility::RandomString(16));

		HWND hWnd = window->hWindow;
		DWORD processId = GetCurrentProcessId();
		DWORD threadId = GetCurrentThreadId();

		std::cout << "Current process id: " << processId << " - Thread id: " << threadId << std::endl;

		HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
			nullptr,
			userDataDirectory.c_str(),
			opt.Get(),
			Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[&window, &environment, &done](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
				{
					environment = env;
					done = true;
					return S_OK;
				}
			).Get()
		);
	});
	while (!done) this_thread::sleep_for(0.1s);

	// Create the webview2 controller
	done = false;
	HWND hWindow = this->window->hWindow;
	this->window->RunAsync([&environment, &hWindow, this, &done] {
		environment->CreateCoreWebView2Controller(hWindow, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
			[this, &done] (HRESULT result, ICoreWebView2Controller * controller) -> HRESULT {
				if (controller == nullptr) {
					cout << "This failed" << endl;
					throw exception("There was an error loading the webview controller");
				}
				this->webview.controller = controller;
				this->webview.controller->get_CoreWebView2(&this->webview.webview);

				ICoreWebView2Settings* Settings;
				this->webview.webview->get_Settings(&Settings);

				Settings->put_IsScriptEnabled(TRUE);
				Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
				Settings->put_IsWebMessageEnabled(TRUE);

				RECT bounds;
				GetClientRect(this->window->hWindow, &bounds);
				this->webview.controller->put_Bounds(bounds);

				done = true;

				return S_OK;
			}
		).Get());
	});
	while (!done) this_thread::sleep_for(0.1s);

	cout << "Done is true" << endl;

	this->webview.environment = environment;
	/*
	this->window->RunAsync([this] {
		this->webview.webview->OpenDevToolsWindow();
	});

	this->window->RunAsync([this] {
		this->webview.webview->Navigate(L"https://google.com");
	});*/

	this->addEventHandlers();

	this->_initialized = true;

	return true;
}

bool Gate::terminate() {
	if (!this->initialized()) throw exception("Gate not initialized");

	this->removeEventHandlers();

	this->webview.controller->Close();
	this->webview.controller = nullptr;
	this->webview.environment = nullptr;
	this->webview.webview = nullptr;

	this->window->StopWindow();
	this->windowThread.join();
	this->window = nullptr;

	return true;
}

bool Gate::navigate(string url) {
	this->window->RunAsync([this, url] {
		this->webview.webview->Navigate(Utility::AS2WS(url).c_str());
	});

	return true;
}

string Gate::addInterceptRule(InterceptOpts &options, std::function<void(ICoreWebView2WebResourceRequestedEventArgs*)> interceptor) {
	cout << "Adding intercept rule" << endl;

	EventRegistrationToken filterToken;
	this->window->RunAsync([this, &filterToken, &options, interceptor] {
		this->webview.webview->AddWebResourceRequestedFilter(Utility::AS2WS(options.Path).c_str(), COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
		
		this->webview.webview->add_WebResourceRequested(
			Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>([options, interceptor](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) {
			
				cout << "Intercept rule triggered" << endl;

				COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext;
				args->get_ResourceContext(&resourceContext);
			
				ICoreWebView2WebResourceRequest* request;
				args->get_Request(&request);
			
				LPWSTR bufRequestedUri;
				request->get_Uri(&bufRequestedUri);

				cout << "URI: " << Utility::WS2AS(bufRequestedUri) << endl;

				if (options.Path == "*" || Utility::WS2AS(bufRequestedUri).rfind(options.Path, 0) == 0) {
					interceptor(args);
					cout << "Done" << endl;
				}

				return S_OK;
			}).Get(),
			&filterToken
		);
	});
	string id = Utility::RandomString(5);
	cout << "Intercept rule ID " << id << " added" << endl;
	this->interceptRemovers.insert(
		make_pair(
			id,
			[this, options, &filterToken] {
				this->window->RunAsync([this, &filterToken, &options] {
					this->webview.webview->RemoveWebResourceRequestedFilter(Utility::AS2WS(options.Path).c_str(), COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
					this->webview.webview->remove_WebResourceRequested(filterToken);
				});
			}
		)
	);

	return id;
}

bool Gate::removeInterceptRule(string ruleId) {
	map<string, std::function<void()>>::iterator it;

	for (it = this->interceptRemovers.begin(); it != this->interceptRemovers.end(); it++)
	{
		if (it->first == ruleId) {
			(it->second)();
			this->interceptRemovers.erase(it);
		}
	}

	return true;
}

bool Gate::removeAllInterceptRules() {
	map<string, std::function<void()>>::iterator it;

	for (it = this->interceptRemovers.begin(); it != this->interceptRemovers.end(); it++)
	{
		(it->second)();
	}

	this->interceptRemovers.clear();

	return true;
}
