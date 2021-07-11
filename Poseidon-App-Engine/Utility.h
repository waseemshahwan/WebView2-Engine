#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <future>
#include "shlwapi.h"
#include "shlobj.h"
#include <memory>
#include <string>
#include <stdexcept>
#include "json11.h"

using namespace std;

class Utility
{
public:
	Utility();
	~Utility();


	static json11::Json ParseJSON(string);
	static LPCWSTR ToLPCWSTR(const std::string& s);
	// Long wide pointer to ascii string 
	static std::string LP2A(LPWSTR base);
	// Ascii string to wide string
	static std::wstring AS2WS(std::string base);
	// Wide String to normal String
	static std::string WS2AS(std::wstring base);
	static std::wstring ReadFile(std::string path);
	static std::vector<unsigned char> ReadFileBytes(std::string path);

	static std::wstring GetAppDataDirectory();
	// Randomization
	static std::string RandomString(unsigned int len);
	static unsigned int RandomNumber(unsigned int min, unsigned int max);
	// Future optimizers
	static std::string WhenAny(std::vector<std::pair<std::string, std::future<bool>>>& eventHandlers, unsigned int durationMs);

	// Encryption/Decryption
	static int encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* key,
		unsigned char* iv, unsigned char* ciphertext);

	static int decrypt(unsigned char* ciphertext, int ciphertext_len, unsigned char* key,
		unsigned char* iv, unsigned char* plaintext);

	// Base64
	static std::string base64_encode(BYTE const* buf, unsigned int bufLen);
	static std::vector<BYTE> base64_decode(std::string const&);

	// Win32 Helpers
	static std::string GetLastErrorAsString();

	// Thread control 
	static HANDLE OpenThreadWrapper(DWORD threadId);
	static DWORD SuspendThreadWrapper(HANDLE threadHandle);
	static bool TerminateThreadWrapper(HANDLE threadHandle);

	// Misc
	static bool CleanProfiles();
	static bool DirectoryExists(std::wstring path);

};

