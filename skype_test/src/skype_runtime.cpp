#include "skype_proto.h"

#include "aes\aes.h" 
#include "base64\base64.h"

#include "..\..\..\skypekit\key.h"

char *CSkypeProto::LoadKeyPair()
{
	HRSRC hRes = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_KEY), L"BIN");
	if (hRes)
	{
		HGLOBAL hResource = LoadResource(g_hInstance, hRes);
		if (hResource)
		{
			aes_context ctx;
			unsigned char key[128];

			int basedecoded = Base64::Decode(MY_KEY, (char *)key, MAX_PATH);
			::aes_set_key(&ctx, key, 128);
			memset(key, 0, sizeof(key));

			basedecoded = ::SizeofResource(g_hInstance, hRes);
			char *pData = (char *)hResource;
			if (!pData)
				return NULL;

			unsigned char *bufD = (unsigned char *)::malloc(basedecoded + 1);
			unsigned char *tmpD = (unsigned char *)::malloc(basedecoded + 1);
			basedecoded = Base64::Decode(pData, (char *)tmpD, basedecoded);

			for (int i = 0; i < basedecoded; i += 16)
				aes_decrypt(&ctx, tmpD+i, bufD+i);

			::free(tmpD);
			bufD[basedecoded] = 0; //cert should be null terminated
			return (char *)bufD;
		}
		return NULL;
	}
	return NULL;
}

int CSkypeProto::StartSkypeRuntime(const wchar_t *profileName)
{
	STARTUPINFO cif = {0};
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;

	wchar_t	fileName[MAX_PATH];
	::GetModuleFileName(g_hInstance, fileName, MAX_PATH);

	wchar_t *skypeKitPath = ::wcsrchr(fileName, '\\');
	if (skypeKitPath != NULL)
		*skypeKitPath = 0;
	::swprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"SkypeKit.exe");

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (::Process32First(snapshot, &entry) == TRUE) {
		while (::Process32Next(snapshot, &entry) == TRUE) {
			if (::wcsicmp(entry.szExeFile, L"SkypeKit.exe") == 0) {
				HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				this->runtimePort += rand() % 8963 + 1000;
				::CloseHandle(hProcess);
				break;
			}
		}
	}
	::CloseHandle(snapshot);

	wchar_t param[128];
	PROCESS_INFORMATION pi;
	VARST dbPath( _T("%miranda_userdata%\\SkypeKit"));
	::swprintf(param, SIZEOF(param), L"-p -P %d -f \"%s\"", this->runtimePort, dbPath);
	int startingrt = ::CreateProcess(
		fileName, param,
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &cif, &pi);

	return startingrt;
}