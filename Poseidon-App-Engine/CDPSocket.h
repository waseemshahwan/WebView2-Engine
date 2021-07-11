#include <cpprest/ws_client.h>
#include <string>
#include "json11.h"

using namespace std;
using namespace web::websockets::client;

typedef function<void(json11::Json)> ListenerCallback;

// JSON RPC spec websocket
class CDPSocket {
	CDPSocket();
private:
	bool _is_connected = false;
	mutex _listening;
	thread* _listen_thread;
	string _uri;

	websocket_client _client;

	map<int, ListenerCallback> _listeners;

	void _listen();
	bool _close();
public:
	~CDPSocket();

	static CDPSocket* Create() { return new CDPSocket; }

	bool connect(string uri);

	bool is_connected() { return this->_is_connected; }
	string uri() { return this->_uri; }
	websocket_client client() { return this->_client; }

	json11::Json request(string method, json11::Json, int timeout = 3);

	void onAny(ListenerCallback);
	void offAny();

	void onId(int, ListenerCallback);
	void offId(int);
};