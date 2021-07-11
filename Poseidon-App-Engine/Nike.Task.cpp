#include "Nike.Task.h"
#include "Gate.h"

NikeTask::NikeTask(Gate* gate, NikeInput input) : Task(gate) {
	if(!this->validateInput(input))
		throw exception("Bad task input");
	
	this->_input = input;
};

NikeTask::~NikeTask() {};

void login() {};

void entry() {};
