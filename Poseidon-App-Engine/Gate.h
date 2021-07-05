#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <map>
#include "WebView2.h"
#include "Window.h"
#include <wrl.h>
#include <wil/result.h>
#include <wil/com.h>

using namespace std;

enum Methods {
	GET,
	POST,
	PUT,
	PATCH,
	DEL,
	NONE
};

struct InterceptOpts {
	Methods Method = NONE;
	string Path;
};

class Gate {
	// Webview related data
	struct Webview {
		wil::com_ptr<ICoreWebView2>            webview;
		wil::com_ptr<ICoreWebView2Environment> environment;
		wil::com_ptr<ICoreWebView2Controller>  controller;
	};

private:

	string _id;
	bool _initialized = false;
	bool _attached = false;
	string _attachedTaskId;
	mutex _locked;

	struct config {
		bool headless = false;
		string proxy = nullptr;
	};

	// id, remover function
	map<string, std::function<void()>> interceptRemovers;
	vector<std::function<void()>> eventRemovers;

	void addEventHandlers();
	void removeEventHandlers();

public:

	// Window & webview related data
	Window* window;
	thread windowThread;
	Webview webview;

	Gate();
	~Gate();

	// getters
	string id() { return this->_id; }
	bool initialized() { return this->_initialized; }
	bool attached() { return this->_attached; }
	string attachedTask() { return this->_attachedTaskId; }

	// initialize the browser and webview2 window
	bool initialize(bool headless, string proxy);

	// Attach / Detach a task (tasks need a gate to operate, and gates shouldnt operate without a task)
	bool attach(string taskId);
	// note: detach should delete all intercept rules
	bool detach();

	// Will detach and kill attached task
	bool terminate();

	// Add intercept rules dynamically, returns intercept rule id (to delete later)
	string addInterceptRule(InterceptOpts &options, std::function<void(ICoreWebView2WebResourceRequestedEventArgs*)> interceptor);
	bool removeInterceptRule (string ruleId);
	bool removeAllInterceptRules();

	bool navigate(string url);

	// Run plain JS code
	auto execute(string code);
};