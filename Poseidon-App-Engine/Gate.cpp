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
#include <string>
#include <exception>
#include <dcomp.h>

using namespace std;

Gate::Gate(bool headless, string proxy) {
	this->_id = (string)Utility::RandomString(16);

	this->initialize(headless, proxy);
}

Gate::~Gate() {
	this->terminate();
}

void Gate::addEventHandlers() {
	// On navigation completed ... do nothing? idk

#ifdef _DEBUG
	EventRegistrationToken frameNavigationCompletedToken;

	this->window->RunAsync([this, &frameNavigationCompletedToken] {

		this->webview.webview2->add_DOMContentLoaded(
			Microsoft::WRL::Callback<ICoreWebView2DOMContentLoadedEventHandler>(
				[this](ICoreWebView2* sender, ICoreWebView2DOMContentLoadedEventArgs* args) -> HRESULT
				{
					string script = "let d=document.createElement('div');d.id='circle';d.style.position='fixed';d.style.zIndex='10000';d.style.width='20px';d.style.height='20px';d.style.pointerEvents='none';d.style.borderRadius='20px';d.style.backgroundColor='rgba(0,0,0,0.5)';document.body.insertBefore(d,document.body.firstChild);window.addEventListener('mousemove',function(e){document.getElementById('circle').style.left=(e.clientX-10)+'px';document.getElementById('circle').style.top=(e.clientY-10)+'px'});";

					this->webview.webview->ExecuteScript(
						Utility::AS2WS(script).c_str(),
						nullptr
					);

					return S_OK;
				}
			).Get(),
			&frameNavigationCompletedToken
		);
	});

	this->eventRemovers.push_back([this, &frameNavigationCompletedToken] {
		this->webview.webview2->remove_DOMContentLoaded(frameNavigationCompletedToken);
	});

#endif

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
			"--no-sandbox",
			"--remote-debugging-address=127.0.0.1",
			"--remote-debugging-port=7728"
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

	wil::com_ptr<ICoreWebView2Environment2> environment2;
	environment->QueryInterface(IID_PPV_ARGS(&environment2));

	wil::com_ptr<ICoreWebView2Environment3> environment3;
	environment2->QueryInterface(IID_PPV_ARGS(&environment3));

	done = false;
	HWND hWindow = this->window->hWindow;
	this->window->RunAsync([&environment3, &hWindow, this, &done] {
		environment3->CreateCoreWebView2Controller(hWindow, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
			[this, &done] (HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
				if (controller == nullptr) {
					cout << "This failed" << endl;
					throw exception("There was an error loading the webview controller");
				}

				this->webview.controller = controller;

				this->webview.controller->get_CoreWebView2(&this->webview.webview);
				wil::com_ptr<ICoreWebView2_2> webview2;
				this->webview.webview->QueryInterface(IID_PPV_ARGS(&webview2));
				this->webview.webview2 = webview2;

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
	this->webview.environment2 = environment2;
	this->webview.environment3 = environment3;
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

	this->removeIntercept();
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

bool Gate::intercept(std::function<HRESULT(ICoreWebView2WebResourceRequestedEventArgs*)> interceptor) {
	cout << "Adding intercept rule" << endl;

	this->interceptFunctor = interceptor;

	EventRegistrationToken filterToken;
	this->window->RunAsync([this, &filterToken] {
		this->webview.webview->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
		
		this->webview.webview->add_WebResourceRequested(
			Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) {
			
				cout << "Intercept rule triggered" << endl;

				COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext;
				args->get_ResourceContext(&resourceContext);
			
				ICoreWebView2WebResourceRequest* request;
				args->get_Request(&request);
			
				LPWSTR bufRequestedUri;
				request->get_Uri(&bufRequestedUri);

				cout << "URI: " << Utility::WS2AS(bufRequestedUri) << endl;

				// rules here
				if (this->interceptFunctor) {
					return this->interceptFunctor(args);
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
			[this, &filterToken] {
				this->window->RunAsync([this, &filterToken] {
					this->webview.webview->RemoveWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
					this->webview.webview->remove_WebResourceRequested(filterToken);
				});
			}
		)
	);

	return true;
}

bool Gate::removeIntercept() {
	map<string, std::function<void()>>::iterator it;

	for (it = this->interceptRemovers.begin(); it != this->interceptRemovers.end(); it++)
	{
		(it->second)();
	}

	this->interceptRemovers.clear();
	this->interceptFunctor = nullptr;

	return true;
}

void Gate::execute(string code) {
	this->window->RunAsync([this, code] {
		this->webview.webview->ExecuteScript(Utility::AS2WS(code).c_str(), NULL);
	});
}

void Gate::waitForNavigation() {
	bool done = false;
	EventRegistrationToken frameNavigationCompletedToken;
	this->window->RunAsync([this, &frameNavigationCompletedToken, &done] {
		this->webview.webview->add_FrameNavigationCompleted(
			Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[this, &done](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
				{
					done = true;
					return S_OK;
				}
			).Get(),
			&frameNavigationCompletedToken
		);
	});

	while (!done) this_thread::sleep_for(0.1s);
	this->webview.webview->remove_FrameNavigationCompleted(frameNavigationCompletedToken);
	return;
}

bool Gate::lock() {
	try {
		this->locker.lock();
	} catch (exception e) {
		cout << "Error with lock: " << e.what() << endl;
		return false;
	}

	return true;
}

// Warning caused by VS is a bug:
// https://developercommunity.visualstudio.com/t/incorrect-lock-warnings-by-analyzer-c26110/670717
bool Gate::unlock() {

	try {
		this->locker.unlock();
	} catch (exception e) {
		cout << e.what() << endl;
		return false;
	}

	return true;
}

