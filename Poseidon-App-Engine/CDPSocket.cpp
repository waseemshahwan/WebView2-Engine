#include "CDPSocket.h";
#include "Utility.h"

CDPSocket::CDPSocket() {};
CDPSocket::~CDPSocket() {
	this->_close();
};

void CDPSocket::_listen() {
	websocket_incoming_message msg;
	try {
		msg = this->client().receive().get();
	} catch (exception e) {
		cout << "The receive failed.. " << e.what() << endl;
		return; // connection closed
	}
	

	json11::Json payload;
	try {
		payload = Utility::ParseJSON(msg.extract_string().get());
	} catch (exception e) {
		cout << "Error! " << e.what() << endl;
		return this->_listen();
	};

	if (!payload.is_object()) return this->_listen();

	if (payload.object_items().count("method") > 0) {
		json11::Json method = payload.object_items().at("method");
		if (method.is_string() && method.string_value() == "Inspector.detached") return; // gate is killed
	};

	int id = 0;
	if (payload.object_items().count("id") > 0) {
		if (payload.object_items().at("id").is_number())
			id = payload.object_items().at("id").int_value();
	}

	for (pair<int, ListenerCallback> item : this->_listeners) {
		if (item.first == id || item.first == -1) {
			item.second(payload);
		}
	}

	return this->_listen();
};

void CDPSocket::onId(int id, ListenerCallback callback) {
	this->_listeners.insert(make_pair(id, callback));
};

void CDPSocket::onAny(ListenerCallback callback) {
	this->_listeners.insert(make_pair(-1, callback));
};

void CDPSocket::offId(int id) {
	this->_listeners.erase(id);
};

void CDPSocket::offAny() {
	this->_listeners.erase(-1);
};

bool CDPSocket::connect(string uri) {
	if (this->_listening.try_lock()) {
		this->_listen_thread = new thread(&CDPSocket::_listen, this);
	} else {
		throw exception("CDPSocket not closed sensibly");
	};
	this->client().connect(Utility::AS2WS(uri)).get();
	this->_is_connected = true;

	return true;
};

bool CDPSocket::_close() {
	this->_is_connected = false;
	this->client().close();

	if (this->_listen_thread != nullptr) {
		this->_listen_thread->join();
		delete this->_listen_thread;
		this->_listening.unlock();
	}

	return true;
};

json11::Json CDPSocket::request(string method, json11::Json params, int timeout) {
	if (!this->is_connected()) throw exception("CDPSocket is closed");

	if (!params.is_object()) throw exception("JSON non-objects not supported");
	
	int id = Utility::RandomNumber(0, 100000);
	json11::Json body = json11::Json::object{
		{ "id", id },
		{ "method", method },
		{ "params", params }
	};

	cout << body.dump() << endl;

	bool recvd = false;
	json11::Json response;
	this->onId(id, [&recvd, &response](json11::Json l_response) {
		recvd = true;
		response = l_response;
	});

	try {
		websocket_outgoing_message outgoing;
		outgoing.set_utf8_message(body.dump());
		this->client().send(outgoing);
	} catch (exception e) {
		throw exception("CDPSocket is closed");
	}

	time_t start = time(0);
	while (!recvd && difftime(time(0), start) < timeout) this_thread::sleep_for(0.1s);

	this->offId(id);

	return response;
}