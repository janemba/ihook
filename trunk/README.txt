ABOUT IHOOK
===========

IHOOK is an easy interface to perform API hooking on Windows operating system.
It is distributed as a static library (.lib) and as a shared library (.dll).
It is also possible to use ihook directly from sources (see dependency).

IHOOK use inline patching technique also known as detour.


DEPENDENCY
============

IHOOK use a disassembler to determine the number of opcode to erased. It is
compiled with BeaEngine source disassembler in a static way.


HOW TO USE
==========

IHOOK provide the following methods :

	hookitByName		// Method performing hook by using name of the function to hook
	hookitByAddress		// Method performing hook by using address of the original function
	getReturnAddressById	// Get the address of the re-entrant execution stream
	getReturnAddressByName	// Idem
	getReturnAddressByAddr	// Idem
	unhookByName		// Cleanup the hook
	unhookById		// Idem
	unhookByAddress		// Idem

see API.txt for a detailed description of the previous methods.

For C coders IHOOK_CALL_STDCALL define must be added if you use IHOOK as a static library.
It is used for the calling convention.

#define IHOOK_CALL_STDCALL
#include "hookit.h"

You don't need it if you use IHOOK as a shared library (.dll).

Moreover, don't forget to use hookit.h file from header directory in your application.


HOW TO COMPILE YOUR PROJECT WITH IHOOK STATIC LIBRARY (libihook.lib)
====================================================================

mingw32-gcc -Wall -O2 -DIHOOK_ENGINE_LIB -c myproject.c -o myproject.o
mingw32-gcc -o myproject.exe myproject.o -s libihook.lib


HOW TO COMPILE YOUR PROJECT WITH IHOOK SHARED LIBRARY (libihook.dll)
====================================================================

mingw32-gcc -Wall -O2 -DIHOOK_ENGINE_DLL -c myproject.c -o myproject.o
mingw32-gcc -o myproject.exe myproject.o -s libihook.dll


HOW TO COMPILE IHOOK SOURCE
===========================

A makefile is provided and it compile IHOOK source with BeaEngine (dependance) source.

To compile IHOOK source as a static library type the following:
	make static

To compile IHOOK source as a shared library type the following:
	make shared


IHOOK source is compiled with the following options:

mingw32-gcc.exe -m32 -O3 -Wextra -Wall -std=c99 -I. -Idep\\beaengine\\include -DBEA_ENGINE_STATIC -c dep\\beaengine\\beaengineSources\\BeaEngine.c -o obj\\Release\\BeaEngine.o
mingw32-gcc.exe -m32 -O3 -Wextra -Wall -std=c99 -I. -Idep\\beaengine\\include -DIHOOK_ENGINE_LIB -DIHOOK_CALL_STDCALL -c hookit.c -o obj\\Release\\hookit.o
ld.exe -r -o libfoo.o obj\\Release\\hookit.o obj\\Release\\BeaEngine.o
ar.exe rcs bin\\libihook.lib libfoo.o

 => libihook.lib


mingw32-gcc.exe -m32 -O3 -Wextra -Wall -std=c99 -I. -Idep\\beaengine\\include -DBEA_ENGINE_STATIC -c dep\\beaengine\\beaengineSources\\BeaEngine.c -o obj\\Release\\BeaEngine.o
mingw32-gcc.exe -m32 -O3 -Wextra -Wall -std=c99 -I. -Idep\\beaengine\\include -DIHOOK_ENGINE_DLL -c hookit.c -o obj\\Release\\hookit.o

ld.exe -r -o libfoo.o obj\\Release\\hookit.o obj\\Release\\BeaEngine.o
mingw32-gcc.exe libfoo.o -s -shared -Wl,--subsystem,windows -o bin\\libihook.dll

 => libihook.dll


A temporary library is generated in order to link BeaEngine (.o) object file statically...

You can also compile ihook (static of shared) with visual studio but some step is needed:
	1) In project Properties -> Configuration Properties -> C/C++
	   In "Additional Include Directories" add the following BeaEngine include path:
		...\dep\beaengine\include

	2) In project Properties -> Configuration Properties -> C/C++ -> Preprocessor
	   In "Preprocessor Definitions" add the following constant:
		IHOOK_ENGINE_LIB
		IHOOK_CALL_STDCALL
		BEA_ENGINE_STATIC  (or BEA_ENGINE_SHARED to compile ihook as a DLL)

	3) Add BeaEngine.c source in the project solution
		src\dep\beaengine\beaengineSources\BeaEngine.c

	4) That's all. You will get a lot of warning due to the use of strcpy function in BeaEngine
	    sources but don't worry this is just warning.


EXAMPLE
=======

typedef int (WINAPI* hook_recv) (SOCKET, const char*, int , int);

int WINAPI   Myrecv(SOCKET s, const char* buf, int len, int flags)
{
  hook_recv  true_recv;
  DWORD      addr;

  printf("I'm in the hook function\n");

  addr = getReturnAddressByName("recv", "ws2_32.dll");

  if ((int) addr < 0)
      printf("If you get here, you're doing something wrong -> %i\n", (int)addr);

  true_recv = (hook_recv) addr;

  return (true_recv(s, buf, len, flags));
}


int	main(void)
{
  int	ret;

  ret = hookitByName("recv", "ws2_32.dll", (DWORD)Myrecv);
  if (ret <= 0)
    printf("Some error here: %i\n", ret);
  unhookById(ret);  // or unhookByName("recv", "ws2_32.dll");

  return (0);
}

SOURCE LAYOUT
=============
	
	ihook
	  |
	  |__ example				// Source file example of ihook
	  |
	  |__ header				// Application-side header to use with ihook library
	  |	|
	  |	|__ hookit.h
	  |
	  |__ src				// ihook source files
	  |	|
	  |	|__ hookit.c
	  |	|
	  |	|__ hookit.h
	  |	|
	  |	|__ Makefile
	  |	|
	  |	|__ bin				// ihook binary library (*_vs = compiled with visual studio)
	  |	     |
	  |	     |__ libihook.lib
	  |	     |
	  |	     |__ libihook.dll
	  |	     |
	  |	     |__ libihook_vs.lib
	  |	     |
	  |	     |__ libihook_vs.dll
	  |
	  |__ dep				// ihook dependency
	  |	|
	  |	|__ beaengine			// beaengine source
	  |	     |
	  |	     |...
	  |
	  |__ API.txt
	  |
	  |__ AUTHORS.txt
	  |
	  |__ CHANGELOG.txt
	  |
	  |__ COPYING.LESSER.txt
	  |
	  |__ COPYING.txt
	  |
	  |__ README.txt
	  |
	  |__ TODO.txt