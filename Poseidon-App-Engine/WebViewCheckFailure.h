#include "WebView2.h"
#include <atlstr.h>
#include <iostream>

#define CHECK_FAILURE_STRINGIFY(arg)         #arg
#define CHECK_FAILURE_FILE_LINE(file, line)  ([](HRESULT hr){ CheckFailure(hr, L"Failure at " CHECK_FAILURE_STRINGIFY(file) L"(" CHECK_FAILURE_STRINGIFY(line) L")"); })
#define CHECK_FAILURE                        CHECK_FAILURE_FILE_LINE(__FILE__, __LINE__)
#define CHECK_FAILURE_BOOL(value)            CHECK_FAILURE((value) ? S_OK : E_UNEXPECTED)

using namespace std;

void ShowFailure(HRESULT hr, CString const& message)
{
    CString text;
    text.Format(L"%s (0x%08X)", (LPCTSTR)message, hr);

    ::MessageBox(nullptr, static_cast<LPCTSTR>(text), L"Failure", MB_OK);
}

bool CheckFailure(HRESULT hr, CString const& message)
{
    if (FAILED(hr)) {
        CString text;
        text.Format(L"%s : 0x%08X", (LPCTSTR)message, hr);

        cout << "WebView2 Error: " << text << endl;

        return true;
    } else return false;
}