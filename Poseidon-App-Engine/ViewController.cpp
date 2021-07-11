#include "ViewController.h"
#include "Utility.h"
#include <ctime>
#include "json11.h"
#include <cmath>

#define RAND_DEC rand() / RAND_MAX

class Document {
public:
	static wstring getElementById(string elementId) {
		ostringstream data;
		data << "let n = window.document.getElementById(\"" << elementId << "\"); console.log(n); n && JSON.stringify({ id: n.id, x: n.getBoundingClientRect().x, y: n.getBoundingClientRect().y, width: n.getBoundingClientRect().width, height: n.getBoundingClientRect().height });";
	
		cout << data.str() << endl;

		return Utility::AS2WS(data.str());
	};

	static wstring querySelector(string query) {
		ostringstream data;
		data << "let n = document.querySelectorAll(\"" << query << "\"); n.length ? Array.from(n).map(i => ({ id: i.id, className: i.className, x: i.getBoundingClientRect().x, y: i.getBoundingClientRect().y, width: i.getBoundingClientRect().width, height: i.getBoundingClientRect().height })) : null;";

		return Utility::AS2WS(data.str());
	}

	static wstring click(double x, double y) {
		
		ostringstream data;

		data << "document.elementFromPoint(" << x << "," << y << ").click();";

		return Utility::AS2WS(data.str());
	}
};

double sqrt3 = sqrt(3);
double sqrt5 = sqrt(5);

void mouseMove(std::function<void(double x, double y)> move, double start_x, double start_y, double dest_x, double dest_y, double G_0 = 9, double W_0 = 0., double M_0 = 15, double D_0 = 12) {
	double current_x = start_x;
	double current_y = start_y;

	double v_x = 0, v_y = 0, W_x = 0, W_y = 0;

	double dist = NULL;
	while (dist >= 1 || dist == NULL) {
		dist = hypot(dest_x - start_x, dest_y - start_y);

		double W_mag = min(W_0, dist);

		if (dist >= D_0) {
			W_x = W_x / sqrt3 + (2 * RAND_DEC - 1) * W_mag / sqrt5;
			W_y = W_y / sqrt3 + (2 * RAND_DEC - 1) * W_mag / sqrt5;
		} else {
			W_x /= sqrt3;
			W_y /= sqrt3;
			if (M_0 < 3)
				M_0 = RAND_DEC * 3 + 3;
			else
				M_0 /= sqrt5;
		}

		v_x += W_x + G_0 * (dest_x - start_x) / dist;
		v_y += W_y + G_0 * (dest_y - start_y) / dist;

		double v_mag = hypot(v_x, v_y);
		if (v_mag > M_0) {
			double v_clip = M_0 / 2 + RAND_DEC * M_0 / 2;
			v_x = (v_x / v_mag) * v_clip;
			v_y = (v_y / v_mag) * v_clip;
		}

		start_x += v_x;
		start_y += v_y;

		int move_x = int(round(start_x));
		int move_y = int(round(start_y));

		if (current_x != move_x || current_y != move_y) {
			current_x = move_x;
			current_y = move_y;

			this_thread::sleep_for(0.1s);

			move(current_x, current_y);
		}
	}

	return;
}

json11::Json ExecuteSync(Gate* gate, string script) {
	bool done = false;
	bool fail = false;
	bool nulled = false;
	json11::Json result;
	gate->window->RunAsync([&gate, &done, &fail, &nulled, &script, &result] {
		wstring* temp = new wstring(Utility::AS2WS(script));
		gate->webview.webview->ExecuteScript(
			(*temp).c_str(),
			Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
				[&done, &fail, &nulled, &result](HRESULT hr, LPCWSTR output) {
					if (hr != S_OK) {
						fail = true;
						done = true;
						cout << "Failed script execution" << endl;
						return S_OK;
					}

					string res = Utility::WS2AS(output);
					if (res == "null") {
						nulled = true;
						done = true;
						return S_OK;
					}

					try {
						result = Utility::ParseJSON(res);
					} catch (exception e) {
						fail = true;
						done = true;
						cout << "Failed JSON parsing" << endl;
						return S_OK;
					}

					done = true;
				}
			).Get()
		);
	});

	while (!done) this_thread::sleep_for(0.05s);
	if (nulled) return json11::Json::NUL;
	if (fail) throw exception("Script execution failed");
	return result;
}

ViewController::ViewController(Gate* gate, CDPSocket* ws_client) {
	this->_gate = gate;
	this->_ws_client = ws_client;
	if (!this->_ws_client->is_connected()) throw exception("CDP connection is not established");
};

ViewController::~ViewController() {
	this->_gate = nullptr;
	delete this->_ws_client;
};

Element ViewController::waitForElement(string elementId, int timeout) {
	cout << "1" << endl;
	this_thread::sleep_for(5s);
	cout << "2" << endl;
	string result;
	cout << "3" << endl;
	bool done = false;
	cout << "4" << endl;
	time_t start = time(0);
	cout << "5" << endl;
	time_t now = time(0);
	cout << "6" << endl;
	while (result.empty() && difftime(now, start) < timeout) {
		cout << "7" << endl;
		now = time(0);
		cout << "8" << endl;
		std::wstring* temp = new wstring(Document::getElementById("input3"));
		this->gate()->window->RunAsync([this, temp, &result, &done] {
			cout << "9" << endl;
			this->gate()->webview.webview->ExecuteScript(
				(*temp).c_str(),
				Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
					[&result, &done](HRESULT hr, LPCWSTR output) -> HRESULT {
						cout << "10" << endl;
						done = true;
						cout << "11" << endl;
						if (hr != S_OK) {
							cout << "Script execution failed" << endl;
							return S_OK;
						}

						cout << "12" << endl;
						result = Utility::WS2AS(output);
						if (result == "null") result = "";
						cout << "Script execution completed: " << result << endl;
						cout << "13" << endl;
						return S_OK;
					}
				).Get()
			);

			delete temp;
		});

		cout << "14" << endl;
		while (!done) this_thread::sleep_for(0.1s);
		cout << "15" << endl;
		cout << "16" << endl;
	}
	 
	cout << "17" << endl;
	if (result.empty()) throw exception("No element found within time constraints");

	cout << "18" << endl;

	json11::Json body = Utility::ParseJSON(result);

	if (!body.is_object()) {
		throw exception("Unexpected response");
	}

	string id = body.object_items().at("id").string_value();
	double x = body.object_items().at("x").number_value();
	double y = body.object_items().at("y").number_value();
	double width = body.object_items().at("width").number_value();
	double height = body.object_items().at("height").number_value();


	Element el = { id, NULL, NULL, x, y, width, height };
	return el;
};

void ViewController::queryElements(string query, vector<Element> &elements) {
	bool done = false;
	string result;

	wstring* temp = new wstring(Document::querySelector(query));

	this->gate()->window->RunAsync([this, temp, &done, &result] {
		this->gate()->webview.webview->ExecuteScript(
			(*temp).c_str(),
			Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
				[&done, &result](HRESULT hr, LPCWSTR output) -> HRESULT {
					done = true;
					if (hr != S_OK) {
						cout << "Script execution failed" << endl;
						return S_OK;
					}

					result = Utility::WS2AS(output);
					if (result == "null") result = "";

					return S_OK;
				}
			).Get()
		);

		delete temp;
	});

	while (!done) this_thread::sleep_for(0.1s);

	if (result.empty()) return;

	json11::Json body = Utility::ParseJSON(result);

	if (!body.is_array()) {
		throw exception("Unexpected response");
	}

	cout << "Found elements:" << endl;

	for (json11::Json item : body.array_items()) {
		string id = item.object_items().at("id").string_value();
		double x = item.object_items().at("x").number_value();
		double y = item.object_items().at("y").number_value();
		double width = item.object_items().at("width").number_value();
		double height = item.object_items().at("height").number_value();

		cout << "making element" << endl;

		Element el = { id, "", "", x, y, width, height};
		elements.push_back(el);

		cout << "pushed" << endl;
	}

	return;

};

bool ViewController::click(Element el) {
	bool done = false;
	bool result = false;
	wstring* temp = new wstring(Document::click(el.x, el.y));
	this->gate()->window->RunAsync([this, &temp, &done, &result] {
		this->gate()->webview.webview->ExecuteScript(
			(*temp).c_str(),
			Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
				[&done, &result](HRESULT hr, LPCWSTR output) -> HRESULT {
					done = true;
					if (hr != S_OK) {
						result = false;
						return S_OK;
					}

					result = true;
					return S_OK;
				}
			).Get()
		);

		delete temp;
	});

	if (!done) this_thread::sleep_for(0.1s);

	return result;
}

bool ViewController::click(int x, int y) {
	bool done = false;
	bool result = false;
	wstring* temp = new wstring(Document::click(x, y));
	this->gate()->window->RunAsync([this, &temp, &done, &result] {
		this->gate()->webview.webview->ExecuteScript(
			(*temp).c_str(),
			Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
				[&done, &result](HRESULT hr, LPCWSTR output) -> HRESULT {
					done = true;
					if (hr != S_OK) {
						result = false;
						return S_OK;
					}

					result = true;
					return S_OK;
				}
			).Get()
		);

		delete temp;
	});

	if (!done) this_thread::sleep_for(0.1s);

	return result;
}

bool ViewController::move(double x, double y) {

	// get current mouse X,Y

	double cur_x, cur_y;
	json11::Json result = ExecuteSync(this->gate(), "");

	// mouseMove()
	return true;
}