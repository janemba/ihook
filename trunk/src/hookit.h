/*
 * This file is part of ihook.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */

#ifndef __HOOKIT_H__
#define __HOOKIT_H__

#if !defined(IHOOK_ENGINE_LIB)
 #if defined(IHOOK_ENGINE_DLL)
  #define IHOOKAPI __declspec(dllexport)
 #else
  #define IHOOKAPI __declspec(dllimport)
 #endif // IHOOK_ENGINE_DLL
#else
 #define IHOOKAPI
#endif // IHOOK_ENGINE_LIB

#if defined(IHOOK_CALL_STDCALL)
 #define IHOOKCALL __stdcall
#else
 #define IHOOKCALL __cdecl
#endif // IHOOK_CALL_STDCALL

#include <windows.h>

enum ERROR_LIST {
    ERR_ALLOCA  = -2,       // Can't allocate memory
    ERR_ADDR    = -3,       // Can't get address of original function
    ERR_PROT    = -4,       // Can't set permission to a page pointed by original function
    ERR_NOTLIST = -5,       // Node referenced by name, id or address not in list (not hooked)
    ERR_PARAM   = -6        // Bad parameters
};


#ifdef __cplusplus
extern "C"
{
#endif //__cpluscplus

/*
** Public API
*/
IHOOKAPI int   IHOOKCALL unhookById(DWORD id);
IHOOKAPI int   IHOOKCALL unhookByName(char *fctname, char *dllname);
IHOOKAPI int   IHOOKCALL unhookByAddress(DWORD hookedAddr);

IHOOKAPI DWORD IHOOKCALL getReturnAddressById(DWORD id);
IHOOKAPI DWORD IHOOKCALL getReturnAddressByName(char *fctname, char *dllname);
IHOOKAPI DWORD IHOOKCALL getReturnAddressByAddr(DWORD hookedAddr);

IHOOKAPI int   IHOOKCALL hookitByAddress(DWORD addrToHook, DWORD hookaddr);
IHOOKAPI int   IHOOKCALL hookitByName(char *fctname, char *dllname, DWORD hookaddr);

#ifdef __cplusplus
}
#endif // __cplusplus

const char __ihook_version__[] = "1.0.2";

#endif /* __HOOKIT_H__ */
