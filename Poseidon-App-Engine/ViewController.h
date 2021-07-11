#include "Gate.h"
#include "Keycodes.h"
#include "CDPSocket.h"

struct Element {
	string id;
	string className;
	string visible;
	double x, y, width, height;
};

class ViewController {
	ViewController(Gate* gate, CDPSocket* ws_client);

private:
	Gate* _gate;
	CDPSocket* _ws_client; // CDP client
public:
	~ViewController();

	static ViewController* Create(Gate* g, CDPSocket* ws) { return new ViewController(g, ws); }

	bool initialized() { return this->_gate != nullptr; }
	Gate* gate() { return this->_gate; }
	CDPSocket* cdp() { return this->_ws_client; }

	// CDP
	bool connect();
	bool is_connected();

	bool move(double, double);
	bool move(Element);
	bool hover(Element);
	bool hover(Element, int speed);
	bool hover(int, int);
	bool hover(int, int, int speed);
	bool click(Element);
	bool click(int x, int y);

	bool type(string text);
	bool typeKey(string keyCode);

	void queryElements(string query, vector<Element> &elements);
	Element waitForElement(string elementId, int timeout);
};