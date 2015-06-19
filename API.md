# VARIABLE LIST #
  * [\_\_ihook\_version\_\_](API#version.md)


---


---

<br />
# NAME #

## ihook\_version ##

# SYNOPSIS #
```
const char __ihook_version__[]
```

# DESCRIPTION #

**ihook\_version** is a string containing API version.

# RETURN VALUE #

**ihook\_version** return version of the library.

# EXAMPLE #
```
extern char __ihook_version__[];

MessageBoxA(NULL, __ihook_version__, "version", MB_OK);
```


---


---


---

<br />
<br />

# METHOD LIST #
  * [hookitByName](API#hookitByName.md)
  * [hookitByAddress](API#hookitByAddress.md)
  * [getReturnAddressByName](API#getReturnAddressByName.md)
  * [getReturnAddressById](API#getReturnAddressById.md)
  * [getReturnAddressByAddr](API#getReturnAddressByAddr.md)
  * [unhookByName](API#unhookByName.md)
  * [unhookById](API#unhookById.md)
  * [unhookByAddress](API#unhookByAddress.md)


---


---

<br />

# NAME #

## hookitByName ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI int IHOOKCALL hookitByName(char *fctname, char *dllname, DWORD hookaddr);
```

# DESCRIPTION #

**hookitByName** function perform an inline patching on the function specified by
_fctname_ which belong to _dllname_. The patch is a short jump to an address
specified by _hookaddr_.

# RETURN VALUE #

**hookitByName** function return a positive value (> 0) if succeed. Otherwise one of the
following values is returned:
<table>
<tr><td>ERR_ALLOCA(-2)</td><td>Can't allocate memory</td></tr>
<tr><td>ERR_ADDR(-3)</td><td>Can't get address of function to hook</td></tr>
<tr><td>ERR_PROT(-4)</td><td>Can't set permission for the page pointed by the function to hook</td></tr>
<tr><td>UNKNOWN_OPCODE(-1)</td><td>BeaEngine return value</td></tr>
<tr><td>OUT_OF_BLOCK(0)</td><td>BeaEngine return value</td></tr>
</table>

_If succeed the value returned by hookitByName is an ID (> 0x1000) and it can be used later to cleanup the hook._

# EXAMPLE #
```
DWORD ret;

ret = hookitByName("recv", "ws2_32.dll", (DWORD)MyHookFct);
```


---


# NAME #

## hookitByAddress ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI int IHOOKCALL hookitByAddress(DWORD addrToHook, DWORD hookaddr);
```

# DESCRIPTION #

**hookitByAddress** function perform an inline patching on the address specified by
_addrToHook_. The patch is a short jump to an address specified by _hookaddr_.

# RETURN VALUE #

**hookitByAddress** function return a positive value (> 0) if succeed. Otherwise one of the
following values is returned:
<table>
<tr><td>ERR_ALLOCA(-2)</td><td>Can't allocate memory</td></tr>
<tr><td>ERR_ADDR(-3)</td><td>Can't get address of function to hook</td></tr>
<tr><td>ERR_PROT(-4)</td><td>Can't set permission for the page pointed by the function to hook</td></tr>
<tr><td>UNKNOWN_OPCODE(-1)</td><td>BeaEngine return value</td></tr>
<tr><td>OUT_OF_BLOCK(0)</td><td>BeaEngine return value</td></tr>
</table>

_If succeed the value returned by hookitByAddress is an ID (> 0x1000) and it can be used later to cleanup the hook._

# EXAMPLE #
```
DWORD ret;

ret = hookitByAddress((DWORD) &function_code, (DWORD) &MyHookFct);
```


---



# NAME #

## getReturnAddressByName ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI DWORD IHOOKCALL getReturnAddressByName(char *fctname, char *dllname);
```

# DESCRIPTION #

**getReturnAddressByName** function return a pointer in order to execute the original
function. To do this, it takes the function name pointed by _fctname_ and the DLL
name which this function belong to.

# RETURN VALUE #

**getReturnAddressByName** function return an address in DWORD format if succeed. This value need to be casted to a function pointer with the same prototype of the function hooked. If failed, the following value is returned:
<table>
<tr><td>ERR_NOTLIST(-5)</td><td>Function not in list (not hooked)</td></tr>
</table>

# EXAMPLE #
```
typedef int (WINAPI* hook_recv) (SOCKET, const char*, int , int);	

DWORD addr;
hook_recv fct;

addr = getReturnAddressByName("recv", "ws2_32.dll");
if ((int) addr < 0)
  printf("You did something wrong\n");		
fct = (hook_recv) addr;
```


---


# NAME #

## getReturnAddressById ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI DWORD IHOOKCALL getReturnAddressById(DWORD id);
```

# DESCRIPTION #

**getReturnAddressById** function return a pointer in order to execute the original
function. To do this, it takes an ID returned by _hookit_ function.

# RETURN VALUE #

**getReturnAddressById** function return an address in DWORD format if succeed. This
value need to be casted to a function pointer with the same prototype of the
function hooked. If failed, the following value is returned:
<table>
<tr><td>ERR_NOTLIST(-5)</td><td>Function not in list (not hooked)</td></tr>
</table>

# EXAMPLE #
```
typedef int (WINAPI* hook_recv) (SOCKET, const char*, int , int);	

DWORD addr;
hook_recv fct;

addr = getReturnAddressById(0x1000);
if ((int) addr < 0)
  printf("You did something wrong\n");		
fct = (hook_recv) addr;
```


---


# NAME #

## getReturnAddressByAddr ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI DWORD IHOOKCALL getReturnAddressByAddr(DWORD addrHooked);
```

# DESCRIPTION #

**getReturnAddressByAddr** function return a pointer in order to execute the original
function. To do this, it takes the address of the original function hooked.

# RETURN VALUE #

**getReturnAddressByAddr** function return an address in DWORD format if succeed. This
value need to be casted to a function pointer with the same prototype of the
function hooked. If failed, the following value is returned:
<table>
<tr><td>ERR_NOTLIST(-5)</td><td>Function not in list (not hooked)</td></tr>
</table>

# EXAMPLE #
```
typedef int (WINAPI* hook_recv) (SOCKET, const char*, int , int);	

DWORD addr;
hook_recv fct;

addr = getReturnAddressByAddr((DWORD) &function_code);
if ((int) addr < 0)
  printf("You did something wrong\n");		
fct = (hook_recv) addr;
```


---



# NAME #

## unhookByName ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI int IHOOKCALL unhookByName(char *fctname, char *dllname);
```

# DESCRIPTION #

**unhookByName** function cleanup the inline patching executed by _hookit_ function. To do this, it takes the function name pointed by _fctname_ and the DLL name which this function belong to.

# RETURN VALUE #

**unhookByName** return TRUE if succeed, otherwise one of the following values is returned:
<table>
<tr><td>ERR_PROT(-4)</td><td>Can't set permission for the page pointed by the function to hook</td></tr>
<tr><td>ERR_NOTLIST(-5)</td><td>Function not in list (not hooked)</td></tr>
<tr><td>ERR_PARAM(-6)</td><td>Bad parameters</td></tr>
</table>


---


# NAME #

## unhookById ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI int IHOOKCALL unhookById(DWORD id);
```

# DESCRIPTION #

**unhookById** function cleanup the inline patching executed by _hookit_ function. To
do this, it takes the ID returned by a previous call to _hookit_.

# RETURN VALUE #

**unhookById** return TRUE if succeed, otherwise one of the following values is returned:
<table>
<tr><td>ERR_PROT(-4)</td><td>Can't set permission for the page pointed by the function to hook</td></tr>
<tr><td>ERR_NOTLIST(-5)</td><td>Function not in list (not hooked)</td></tr>
</table>

---


# NAME #

## unhookByAddress ##

# SYNOPSIS #
```
#define IHOOK_CALL_STDCALL
#include "hookit.h"

IHOOKAPI int IHOOKCALL unhookByAddress(DWORD addrHooked);
```

# DESCRIPTION #

**unhookByAddress** function cleanup the inline patching executed by _hookitByAddress_ function. To
do this, it takes the address of the original function hooked.

# RETURN VALUE #

**unhookByAddr** return TRUE if succeed, otherwise one of the following values is returned:
<table>
<tr><td>ERR_PROT(-4)</td><td>Can't set permission for the page pointed by the function to hook</td></tr>
<tr><td>ERR_NOTLIST(-5)</td><td>Function not in list (not hooked)</td></tr>
</table>