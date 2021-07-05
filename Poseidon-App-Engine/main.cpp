#include <iostream>
#include "Gate.h"
#include "Utility.h"

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

using namespace std;

int main(void) {

	Utility::CleanProfiles();

	/*while (true) {
		for (int i = 0; i < 50; i++) {
			thread t = thread([] {
				cout << "Started thread: " << this_thread::get_id() << endl;
				Gate* gate = new Gate();
				if (!gate->initialize(true, "")) {
					cout << "Failed" << endl;
				}
				else cout << "Success" << endl;

				this_thread::sleep_for(3s);

				gate->terminate();
				delete gate;
			});

			t.detach();

		};
		this_thread::sleep_for(15s);
	}*/

	Gate* gate = new Gate();
	gate->initialize(false, "");

	InterceptOpts options{};

	options.Path = "*";

	gate->addInterceptRule(options, [](ICoreWebView2WebResourceRequestedEventArgs* args) {
		cout << "HEllo World!" << endl;
	});
	bool success = gate->navigate("https://google.com/");
	cout << "Success of nav: " << success << endl;

	cin.get();

	gate->terminate();
	delete gate;

	cin.get();


	return 0;
}

/*

			cout << "INTERCEPTED!!!!!" << endl;

			COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext;
			args->get_ResourceContext(&resourceContext);

			ICoreWebView2WebResourceRequest* request;
			args->get_Request(&request);

			LPWSTR bufRequestedUri;
			request->get_Uri(&bufRequestedUri);

			wil::com_ptr<ICoreWebView2_2> webview2;
			wil::com_ptr<ICoreWebView2Environment> environment;
			wil::com_ptr<ICoreWebView2WebResourceResponse> response;

			gate->webview.webview->QueryInterface(IID_PPV_ARGS(&webview2));
			webview2->get_Environment(&environment);

			string resp = "<h1>HELLO WORLD!!!!</h1>";

			BYTE* tByte = new BYTE[resp.length()];
			memcpy(tByte, resp.c_str(), resp.length());

			wil::com_ptr<IStream> stream = SHCreateMemStream(tByte, resp.length());
			environment->CreateWebResourceResponse(stream.get(), 200, nullptr, L"Content-Type: application/json", &response);
			args->put_Response(response.get());
			args->put_Response(response.get());
			stream->Release();
			request->Release();
			return;
*/