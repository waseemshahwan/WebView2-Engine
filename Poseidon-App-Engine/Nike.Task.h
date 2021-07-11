#include "Task.h"
#include "Gate.h"

struct NikeAccount {
	string username;
	string password;
};

struct NikeProduct {
	string SKU;
	string url;
};

struct NikeInput : TaskInput {
	NikeAccount account;
	NikeProduct product;
};

class NikeTask : public Task {

private:

	bool validateInput(NikeInput input) {
		if (
			input.account.username.empty() ||
			input.account.password.empty() ||
			( input.product.SKU.empty() && input.product.url.empty() )
			) return false;
		else {
			// do more validation here
			return true;
		}
	}

	NikeInput _input;

public:

	NikeTask(Gate* gate, NikeInput input);
	~NikeTask();

	NikeInput input() { return this->_input; }

	void login();
	void entry();
};