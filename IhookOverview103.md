
# Introduction #

All the tests was made with Visual Studio express 2010 and mingw32++ (and optionnaly Code::Blocks).


# Compiling ihook project #

## Compiling ihook with Visual Studio ##

Some steps is required in order to compile ihook project with Visual Studio. First of all the latest sources must be downloaded. Once done, open Visual Studio and create a new project as _Win32 Project_, click _next_, select _Static library_ and unselect _Precompiled header_.

Next add the following files to the project:
  * hookit.c
  * hookit.h
  * BeaEngine.c (from ...\dep\beaengine\beaengineSources\BeaEngine.c)

To finish, _BeaEngine_ header must be added:
  * Go to project properties -> _Configuration Properties_ -> _C/C++_ -> _General_
  * Select _Additional Include Directories_ -> _edit_ and add the following directory:
    * _...\dep\beaengine\include_

Now, for the last step some directive for the preprocessor must be added:
  * Go to project properties -> _Configuration Properties_ -> _C/C++_ -> _Preprocessor_
  * Select _Preprocessor Definitions_ -> _edit_ and add the following constants (one per line):
    * _IHOOK\_CALL\_STDCALL_
    * _IHOOK\_ENGINE\_LIB_
    * _BEA\_ENGINE\_STATIC_

And voilà ! The project is ready to compile but you will get a lot of warning due to the use of _strcpy_ function. Don't worry :)

To compile ihook as a shared library (DLL file), just create a _Win32 Project_ and select _DLL_, _next_ and select _Empty Project_.

Next, the previous last step need to be modified. Only the following constant is required:
  * _IHOOK\_ENGINE\_DLL_
  * _BEA\_ENGINE\_STATIC_



## Compiling ihook with Code::Blocks ##

A _Makefile_ is provided to compile ihook with C::B. Here is the step required to compile ihook project:
  1. Create a _static library_ project.
  1. Copy _hookit.c_, _hookit.h_, _Makefile_ and _dep_ directory into the project directory.
  1. Add _hookit.c_ and _hookit.h_ in the project (_Right-click on the project name and select Add -> Existing files_).
  1. Go to the properties of the project and click to _Project settings_ tab.
  1. Select _This is a custom Makefile_.

First step done. Now C::B need to know which rules to invoke for the makefile. Here is the required steps:
  1. In project properties click on _Build target_ tab and click to _Build options_ button.
  1. Select _"Make" commands_ tab.
  1. insert the following:
```
$make -f $makefile ihook
$make -f $makefile $file
$make -f $makefile clean
$make -q -f $makefile clean
```


## Compiling ihook with MinGW ##

Go to the source (_src_) directory and just type the following command:
```
make ihook
```


# Using ihook static library #

## Example ##

This example is the same as the previous library version (with small modifications). It simply hook _httpOpenRequestA_ function from _wininet.dll_ library. Next, the _objectName_ parameter is writing in _C:\test\_log.txt_. Moreover, a function that just call _MessageBox_ function is hooked to demonstrate to ability to hook everywhere. In fact, the API function used to do this is not limited to function hooking. It is what I called **code hooking**.

In this example two hook type is used :
  1. Hook on a Windows API function : _httpOpenRequestA_ from _wininet.dll_
  1. Hook on a function code : function inside the DLL

After, the DLL file was generated it can be injected using **RemoteDLL**.

### `main.cpp` ###
```
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
            id = hookitByAddress((DWORD) &nothing, (DWORD) &hook_nothing); // or hookitByAddress((DWORD) &nothing + 5, (DWORD) &hook_nothing)
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
```

### `hookit.h` ###
```
File from ihook package in header directory.
```

## Compiling example with Visual Studio ##

  1. Create an empty DLL project
  1. Add _main.cpp_ and _hookit.h_ file to the project. Don't forget to copy libihook\_vs.lib file in the same directory that source files.
  1. Go to project properties -> _Configuration Properties_ -> _C/C++_ -> _Preprocessor_
    * Select _Preprocessor Definitions_ -> _edit_ and add the following constant: _IHOOK\_ENGINE\_LIB_
  1. Compile.

![http://ihook.googlecode.com/svn/wiki/vs.png](http://ihook.googlecode.com/svn/wiki/vs.png)

## Compiling example with Code::Blocks ##

  1. Create a _shared library_ project
  1. Add _main.cpp_ and _hookit.h_ file to the project. Don't forget to copy _libihook.lib_ file in the same source files directory.
  1. Go to Build Options -> select _Release_ on the left pane -> _Compiler settings_ -> _#defines_ -> add the following:
    * _IHOOK\_ENGINE\_LIB_
  1. _Linker settings_ -> in _other link options_ pane add the following:
    * _-L. -llibihook
  1. Compile._

![http://ihook.googlecode.com/svn/wiki/cb.png](http://ihook.googlecode.com/svn/wiki/cb.png)

## Compiling example with MinGW ##

  1. Go to _example_ directory from ihook package
  1. Type the following:
```
C:\Dev\ihook\example>mingw32-gcc -O2 -Wall -DIHOOK_ENGINE_LIB -c main.cpp -o main.o
C:\Dev\ihook\example>mingw32-gcc -shared -o test.dll main.o libihook.lib
```