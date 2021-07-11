#include <WinSock2.h> // needs to be included before WebView2.h to prevent include collisions
//#include "ViewController.h"

//typedef websocketpp::client<websocketpp::config::asio_client> client;
//typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

/*

Gate: Browser instance, can be used by a task.
	Requirements:
		- Able to be created or destroyed on demand.
		- A deletion should detach and terminate attached task.
		- Able to run JS code on the browser and check for errors.

Task: Thread which controls browser to solve CAPTCHAs.
	Requirements:
		- Able to be terminated and detached from gate
		- Runs asynchronously
		- Abstract class to allow for multiple types of CAPTCHAs or tasks.

*/

int main() {



	return 0;
};