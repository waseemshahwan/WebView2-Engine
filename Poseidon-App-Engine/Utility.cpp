#include "Utility.h"
#include <random>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "base64.h"
#include <iostream>

std::random_device rd;
std::mt19937 mt(rd());

std::string Utility::LP2A(LPWSTR base) {
	std::wstring a = &base[0];
	std::string b(a.begin(), a.end());
	return b;
}

LPCWSTR Utility::ToLPCWSTR(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r.c_str();
}

std::wstring Utility::AS2WS(std::string base)
{
	return std::wstring(base.begin(), base.end());
}

std::string Utility::WS2AS(std::wstring base)
{
	return std::string(base.begin(), base.end());
}

std::wstring Utility::ReadFile(std::string path) {
	std::ifstream readSetup(path);
	std::string setupScript{ "" };
	std::string line = "";
	while (std::getline(readSetup, line)) {
		setupScript += line + "\n";
	}
	std::wstring wsSetupScript(setupScript.begin(), setupScript.end());
	return wsSetupScript;
}

std::vector<unsigned char> Utility::ReadFileBytes(std::string path)
{
	const std::string inputFile = path;
	std::ifstream inFile(inputFile, std::ios_base::binary);

	inFile.seekg(0, std::ios_base::end);
	size_t length = inFile.tellg();
	inFile.seekg(0, std::ios_base::beg);

	std::vector<char> buffer;
	buffer.reserve(length);
	std::copy(std::istreambuf_iterator<char>(inFile),
		std::istreambuf_iterator<char>(),
		std::back_inserter(buffer));

	return std::vector<unsigned char>(buffer.begin(), buffer.end());
}

std::wstring Utility::GetAppDataDirectory()
{

	WCHAR s_title[] = { 0 };
	TCHAR path[MAX_PATH];
	std::wstring dataDirectory;
	HRESULT hr = SHGetFolderPath(nullptr, CSIDL_APPDATA, NULL, 0, path);
	if (SUCCEEDED(hr))
	{
		dataDirectory = std::wstring(path);
		dataDirectory.append(L"\\Microsoft\\");
	}
	else
	{
		dataDirectory = std::wstring(L".\\");
	}

	dataDirectory.append(L"PoseidonData");
	return dataDirectory;
}

std::string Utility::RandomString(unsigned int len) {

	std::string randomCharacters{ "abcdef123456789" };
	std::random_device randomDevice;
	std::mt19937 generator(randomDevice());
	std::uniform_int_distribution<> distribution(0, randomCharacters.size() - 1);

	std::string randomString{};

	for (unsigned int x = 0; x < len; x++) {
		randomString += randomCharacters[distribution(generator)];
	}

	return randomString;

}

void handleErrors() {
	std::cout << "ERROROR" << std::endl;
}

unsigned int Utility::RandomNumber(unsigned int min, unsigned int max) {
	std::uniform_int_distribution<unsigned int> dist(min, max);
	return dist(mt);
}

std::string Utility::WhenAny(std::vector<std::pair<std::string, std::future<bool>>>& eventHandlers, unsigned int durationMs)
{

	const unsigned int PollSpeed{ 100 };

	for (unsigned int x = 0; x < durationMs / PollSpeed; x++) {

		for (auto& eventEntry : eventHandlers) {
			if (eventEntry.second._Is_ready()) {
				eventEntry.second.get();
				return eventEntry.first;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(PollSpeed));
	}

	return "";
}


int Utility::encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* key,
	unsigned char* iv, unsigned char* ciphertext)
{
	EVP_CIPHER_CTX* ctx;

	int len;

	int ciphertext_len;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();

	/*
	 * Initialise the encryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * In this example we are using 256 bit AES (i.e. a 256 bit key). The
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits
	 */
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		handleErrors();

	/*
	 * Provide the message to be encrypted, and obtain the encrypted output.
	 * EVP_EncryptUpdate can be called multiple times if necessary
	 */
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();
	ciphertext_len = len;

	/*
	 * Finalise the encryption. Further ciphertext bytes may be written at
	 * this stage.
	 */
	if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
		handleErrors();
	ciphertext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return ciphertext_len;
}


int Utility::decrypt(unsigned char* ciphertext, int ciphertext_len, unsigned char* key,
	unsigned char* iv, unsigned char* plaintext)
{
	EVP_CIPHER_CTX* ctx;

	int len;

	int plaintext_len;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();

	/*
	 * Initialise the decryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * In this example we are using 256 bit AES (i.e. a 256 bit key). The
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits
	 */
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		handleErrors();

	/*
	 * Provide the message to be decrypted, and obtain the plaintext output.
	 * EVP_DecryptUpdate can be called multiple times if necessary.
	 */
	if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;

	/*
	 * Finalise the decryption. Further plaintext bytes may be written at
	 * this stage.
	 */
	if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
		handleErrors();
	plaintext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return plaintext_len;
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(BYTE c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string Utility::base64_encode(BYTE const* buf, unsigned int bufLen) {
	std::string ret;
	int i = 0;
	int j = 0;
	BYTE char_array_3[3];
	BYTE char_array_4[4];

	while (bufLen--) {
		char_array_3[i++] = *(buf++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}

std::vector<BYTE> Utility::base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	BYTE char_array_4[4], char_array_3[3];
	std::vector<BYTE> ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}

	return ret;
}

std::string Utility::GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

HANDLE Utility::OpenThreadWrapper(DWORD threadId) {
	HANDLE oHandle = OpenThread(
		THREAD_TERMINATE | THREAD_SUSPEND_RESUME,
		false,
		threadId
	);
	return oHandle;
}

DWORD Utility::SuspendThreadWrapper(HANDLE threadHandle) {
	DWORD sResult = SuspendThread(threadHandle);
	return sResult;
}

bool Utility::TerminateThreadWrapper(HANDLE threadHandle) {
	bool tResult = TerminateThread(threadHandle, 1);
	return tResult;
}

bool Utility::DirectoryExists(std::wstring path) {
	DWORD dwAttrib = GetFileAttributesW(path.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool Utility::CleanProfiles() {
	std::wstring dirLocation = Utility::GetAppDataDirectory();

	if (Utility::DirectoryExists(dirLocation)) {

		SHFILEOPSTRUCT file_op = {
			NULL,
			FO_DELETE,
			dirLocation.c_str(),
			L"",
			FOF_NOCONFIRMATION |
			FOF_NOERRORUI |
			FOF_SILENT,
			false,
			0,
			L""
		};
		SHFileOperation(&file_op);

	}

	bool mkDir = CreateDirectoryW(
		dirLocation.c_str(),
		NULL
	);

	return true;
}