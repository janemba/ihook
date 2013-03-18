#include <stdio.h>
#include <windows.h>
#include <wininet.h>

#define IHOOK_CALL_STDCALL
#include "hookit.h"


#if defined(_MSC_VER)
 #pragma comment(lib, "wininet.lib")
 #pragma comment(lib, "libihook_vs.lib")
#endif /* _MSC_VER */


typedef HINTERNET (WINAPI *h_OpenReq) (HINTERNET, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, DWORD, DWORD_PTR);
typedef int (*h_not) (const char*);

static void writeToLog(const char *msg)
{
    HANDLE hFile;
    DWORD dwBytesWritten = 0;

    hFile = CreateFile(TEXT("C:\\test_log.txt"),
                       FILE_APPEND_DATA,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        MessageBoxA(NULL, "CreateFile", "ERROR", MB_OK);
    else {
        WriteFile(hFile, msg, strlen(msg), &dwBytesWritten, NULL);
        WriteFile(hFile, "\n", 1, &dwBytesWritten, NULL);
        CloseHandle(hFile);
    }
}

HINTERNET WINAPI hook_OpenRequest(HINTERNET hConnect,
                                  LPCTSTR verb,
                                  LPCTSTR objectName,
                                  LPCTSTR version,
                                  LPCTSTR referer,
                                  LPCTSTR acceptTypes,
                                  DWORD dwFlags,
                                  DWORD_PTR dwContext)
{
    h_OpenReq   fct;
    DWORD       addr;


    writeToLog((const char*) objectName);

    addr = getReturnAddressByName((char*)"HttpOpenRequestA", (char*)"wininet.dll");
    fct  = (h_OpenReq) addr;

    return (fct(hConnect, verb, objectName, version, referer, acceptTypes, dwFlags, dwContext));
}

static int  nothing(const char *msg)
{
    MessageBoxA(NULL, msg, "I'm in nothing function", MB_OK);
    return (0);
}

static int  hook_nothing(const char *msg)
{
    DWORD addr;
    h_not fct;

    MessageBoxA(NULL, msg, "HOOKED", MB_OK);

    addr = getReturnAddressByAddr((DWORD) &nothing);
    fct  = (h_not) addr;

    return (fct(msg));
}



extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    DWORD   id = 0;
    extern const char __ihook_version__[];

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
			writeToLog(__ihook_version__);
            id = hookitByAddress((DWORD) &nothing, (DWORD) &hook_nothing);
            if (id <= 0)
            {
                writeToLog("Something goes wrong, check returned value");
                return -1;
            }

            id = hookitByName((char*)"HttpOpenRequestA", (char*)"wininet.dll", (DWORD) hook_OpenRequest);
            if (id <= 0) {
                writeToLog("Something goes wrong, check returned value");
                return -2;
            }

            nothing("hook function code");

            break;

        case DLL_PROCESS_DETACH:
            unhookByName((char *) "HttpOpenRequestA", (char*) "wininet.dll");
            unhookByAddress((DWORD) &nothing);
            break;
    }
    return TRUE;
}
