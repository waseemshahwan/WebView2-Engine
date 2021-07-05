#include <iostream>
#include <string>
#include <thread>
#include "Gate.h"

using namespace std;

class Task {
private:
	string _id;
	Gate _gate;

	bool _attached = false;
public:
	Task();
	~Task();
	
	// getters
	string id() { return this->_id; }
	Gate gate() { return this->_gate; }
	bool attached() { return this->_attached; }

	// Attach to a gate (cannot operate without a gate)
	bool attach(Gate*);
	bool detach();
};