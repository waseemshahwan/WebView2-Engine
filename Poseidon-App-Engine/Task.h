#include <iostream>
#include <string>
#include <thread>
#include "Gate.h"
#include "Utility.h"
#include "ViewController.h"
#include "ExecuteAround.h"

using namespace std;

struct TaskInput {};

class Task {

private:
	string _id;
	Gate* _gate;

	bool _attached = false;

	ExecuteAround<ViewController, shared_ptr, function<void()>, function<void()>>* controller;

	void _before() {
		if (this->controller == nullptr) {
			cout << "ViewController is broken!" << endl;
		}
	};
	void _after() {
	};

public:
	Task(Gate* gate, CDPSocket* ws_client) {
		this->_id = Utility::RandomString(16);
		this->attach(gate);
		ExecuteAround<ViewController, shared_ptr, function<void()>, function<void()>>
			vc(ViewController::Create(gate, ws_client), bind(&Task::_before, this), bind(&Task::_after, this));
		this->controller = &vc;
	};

	virtual ~Task() {
		this->detach();
		delete this->controller;
	};
	
	// getters
	string id() { return this->_id; }
	Gate* gate() { return this->_gate; }
	bool attached() { return this->_attached; }

	// Attach to a gate (cannot operate without a gate)
	bool attach(Gate* gate) {
		if (gate->lock()) {
			this->_gate = gate;
			this->_attached = true;
		} else return false;

		return true;
	};

	bool detach() {
		if (this->attached() && this->gate()->unlock()) {
			this->_gate = nullptr;
			this->_attached = false;
		} else return false;

		return true;
	};
};