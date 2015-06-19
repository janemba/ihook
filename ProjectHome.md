
# NEWS #
**03/18/13
  * Usable with Visual Studio (see [ihook overview](IhookOverview103.md))** 03/13/13
  * New variable in API (library version in ihook\_version string see [API](API.md))
  * Bug correction
**02/26/13
  * New API
  * Can perform function code hooking (hook everywhere)
  * Bug correction**

# IHOOK #
IHOOK is entirely written in pure C. It is distributed as a static/shared library. It provide an easy interface to perform API hooking and function code hooking on Windows operating system in C/C++.

ihook is developed using Code::Blocks IDE and mingw32 compiler.

# DEPENDENCY #
IHOOK use a disassembler to determine the number of opcode to erased. It is
compiled with BeaEngine source disassembler in a static way.

# HOW TO USE #
## OVERVIEW ##
IHOOK provide the following methods :

  * hookitByName - Method performing the hook
  * hookitByAddress - Idem
  * getReturnAddressById - Get the address of the re-entrant execution stream
  * getReturnAddressByName - Idem
  * getReturnAddressByAddr - Idem
  * unhookByName - Cleanup the hook
  * unhookById - Idem
  * unhookByAddress - Idem


see API.txt for a detailed description of the previous methods.

For C coders IHOOK\_CALL\_STDCALL define must be added if you use IHOOK as a static library.
It is used for the calling convention.

```
#define IHOOK_CALL_STDCALL
#include "hookit.h"
```

You don't need this define if you use IHOOK as a shared library (.dll).

## USING IHOOK ##
IHOOK can be used for four reasons:
  * In a DLL for DLL injection.
  * In integration with others project.
  * For learning how API hooking works.
  * In your own binary (not sure it is useful).

## HOW TO COMPILE AS A STATIC LIBRARY ##
```
mingw32-gcc -Wall -O2 -DIHOOK_ENGINE_LIB -c myproject.c -o myproject.o
mingw32-gcc -o myproject.exe myproject.o -s libihook.lib
```

## HOW TO COMPILE AS A SHARED LIBRARY ##
```
mingw32-gcc -Wall -O2 -DIHOOK_ENGINE_DLL -c myproject.c -o myproject.o
mingw32-gcc -o myproject.exe myproject.o -s libihook.dll
```

# WHY USING IT ? #
  * Monitoring / Tracing / Spying / Fuzzing / Debugging
  * Easy distributable license
  * You don't like detours license
  * Easy to use
  * Clean code


# EXAMPLE #
```
typedef int (WINAPI* hook_recv) (SOCKET, const char*, int , int);

int WINAPI   Myrecv(SOCKET s, const char* buf, int len, int flags)
{
  hook_recv  true_recv;
  DWORD      addr;

  printf("I'm in the hook function");

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
```

_see the wiki for detailed example_ ([ihook overview](IhookOverview101.md), [API](API.md))

# TODO #

  * 64 bit version