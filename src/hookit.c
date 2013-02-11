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

#include <windows.h>
#include <string.h>

#define BEA_ENGINE_STATIC
#include <beaengine\BeaEngine.h>

#include "hookit.h"

/*
** Opcode for jmp/nop
*/
#define JMP_OPCODE 0xe9
#define NOP_OPCODE 0x90

/*
** Size of jmp instruction
** random index for node ID
*/
#define JMP_SIZE   0x05
#define INDEX_ID   0x1000

/*
** Definition of hook meta-information node
*/
typedef struct      node_s
{
    char            *fctname;       // Name of the function to hook
    char            *dllname;       // Name of the DLL which function belong to
    BYTE            *erased;        // Backup of bytes erased
    BYTE            *returned;      // Opcode for returning to the original execution stream
    DWORD           addrHooked;     // Address of the original function to hook
    DWORD           nodeid;         // Identifier of the node
    struct node_s   *next;          // Next node on the list
}                   node_t;

/*
** Global variable of nodes list
*/
static struct node_s *hookList_gl = NULL;


/****
** PRIVATE API
*/


/*
** DESC   : Create hook meta-information node and add it to the list.
** INPUT  :
**          _in char* : Name of function to hook
**          _in char* : Name of the DLL which function to hook belong to
**          _in BYTE* : Backup of erased opcode
**          _in DWORD : 32-bit address of the function to hook
** OUTPUT : If succeed return a node ID (> 0x1000) otherwise return FALSE
*/
static inline DWORD             _addHookToList(char *fctname, char *dllname, BYTE *opcode, DWORD addrToHook)
{
    node_t                      *hooknode_s;
    BYTE                        *returned;
    int                         len_erased;
    int                         jmp;

    jmp = JMP_OPCODE;
    len_erased = strlen((char*) opcode);

    hooknode_s = (node_t*) malloc(sizeof(node_t));
    if (! hooknode_s)
        return (FALSE);

    returned = (BYTE *) malloc((sizeof(BYTE) * len_erased) + JMP_SIZE);
    if (! returned)
        return (FALSE);

    ZeroMemory((PVOID) hooknode_s, sizeof(node_t));
    ZeroMemory((PVOID) returned, (sizeof(BYTE) * len_erased + JMP_SIZE));

    memcpy(returned, opcode, len_erased);
    memcpy(returned + len_erased, &jmp, 1);
    * ((DWORD* ) (&returned[len_erased + 1 ]) ) = addrToHook - (DWORD) returned - JMP_SIZE;

    hooknode_s->fctname    = fctname;
    hooknode_s->dllname    = dllname;
    hooknode_s->erased     = opcode;
    hooknode_s->addrHooked = addrToHook;
    hooknode_s->returned   = returned;

    if (! hookList_gl)
    {
        hooknode_s->nodeid = INDEX_ID;
        hookList_gl = hooknode_s;
    }
    else
    {
        static int idx = 0;
        node_t     *move_s;

        idx    = 1;
        move_s = hookList_gl;
        while (move_s->next)
        {
            move_s = move_s->next;
            idx++;
        }
        hooknode_s->nodeid = INDEX_ID + idx;
        move_s->next = hooknode_s;
    }

    return (hooknode_s->nodeid);
}

/*
** DESC   : Set rwx permission on a specified page if necessary.
** INPUT  :
**          _in  DWORD : 32-bit address of the function to hook
**          _out DWORD*: Address of the variable to get old protection (see. msdn/VirtualProtect)
** OUTPUT : If succeed return TRUE otherwise return FALSE
*/
static BOOL                      _setPermission(DWORD addrToHook, DWORD *protect)
{
    MEMORY_BASIC_INFORMATION    lpBuffer;

    VirtualQuery((LPCVOID) addrToHook, &lpBuffer, sizeof(MEMORY_BASIC_INFORMATION));
    if (lpBuffer.AllocationProtect != PAGE_EXECUTE_READWRITE)
    {
        BOOL  ret;
        DWORD prot;

        ret = VirtualProtect((LPVOID) addrToHook,
                             0x1000,
                             PAGE_EXECUTE_READWRITE,
                             &prot);
        if (! ret)
            return (FALSE);

        *protect = prot;
    }

    return (TRUE);
}


/*
** DESC   : Set old permission on a specified page if necessary.
** INPUT  :
**          _in  DWORD : 32-bit address of the function to hook
**          _int DWORD : Address of the variable to get old protection (see. msdn/VirtualProtect)
** OUTPUT : None
*/
static void                      _unsetPermission(DWORD addrToHook, DWORD protect)
{
    if (protect)
        VirtualProtect((LPVOID) addrToHook, JMP_SIZE, protect, &protect);
}


/*
** DESC   : Unset rwx permission if set before and clean-up trampoline.
** INPUT  :
**          _in  DWORD : 32-bit address of the function hooked
**          _in  BYTE* : Opcode erased
** OUTPUT : If succeed return TRUE otherwise return FALSE
*/
static BOOL                     _unsetHook(DWORD addrHooked, BYTE *opcode)
{
    size_t                      cnt;
    size_t                      len;
    DWORD                       oldProtect;

    len = strlen((char *) opcode);

    oldProtect = 0;
    if (! _setPermission(addrHooked, &oldProtect))
        return (FALSE);

    cnt = 0;
    while (cnt < len)
    {
        * ((BYTE *) (addrHooked + cnt)) = opcode[cnt];
        cnt++;
    }

    _unsetPermission(addrHooked, oldProtect);

    return (TRUE);
}

/*
** DESC   : Unset node from the global list and free the allocated data.
** INPUT  :
**          _in node_t* : Node to release
**          _in node_t* : Previous node
** OUTPUT : None
*/
static void                     _unsetNode(node_t *cur_node, node_t *prev_node)
{
    if (prev_node)
        prev_node->next = cur_node->next;

    free(cur_node->erased);
    free(cur_node->returned);
    free(cur_node);
}

/*
** DESC   : Get the size of byte to be erased from the address of the
**          function top hook.
** INPUT  :
**          _in  DWORD : 32-bit address of the function to hook
** OUTPUT : If succeed return the length of byte to be erased otherwise
**          return a value <= 0.
*/
static inline int                      _getLDE(DWORD addrToHook)
{
    int                         length, limit;
    DISASM                      dAsm;

    memset(&dAsm, 0, sizeof(DISASM));

    length = 0;
    limit  = (int) (JMP_SIZE + sizeof(addrToHook));
    while (length < limit)
    {
        dAsm.EIP = addrToHook + length;
        length += Disasm(&dAsm);

        if (length == UNKNOWN_OPCODE ||length == OUT_OF_BLOCK)
            return (length);
    }
    return (length);
}

/*
** DESC   : Write a trampoline at the desired address.
** INPUT  :
**          _in DWORD : 32-bit address of the function to hook
**          _in DWORD : 32-bit address of the hook function
**          _in int   :Length of byte to erased
** OUTPUT : If succeed return BYTE array of erased opcode otherwise return
**          NULL.
*/
static inline BYTE*             _writeDetour(DWORD addrToHook, DWORD hookAddr, int length)
{
    int                         cnt, offset;
    BYTE                        *opcode;

    opcode = (BYTE *) malloc(length + 1);
    if (! opcode)
        return (NULL);

    ZeroMemory((PVOID) opcode, length + 1);
    for (cnt = 0; cnt < length; cnt++)
        opcode[cnt] = ((BYTE *) addrToHook)[cnt];

    for (cnt = 0; cnt < length; cnt++)
        * ((BYTE *) (addrToHook + cnt)) = NOP_OPCODE;

    offset = hookAddr - addrToHook - JMP_SIZE;

    * ((BYTE *) (addrToHook))      = JMP_OPCODE;
    * ((DWORD *) (addrToHook + 1)) = offset;

    return (opcode);
}


/*
** DESC   : Get the address of a function.
** INPUT  :
**          _in char* : Name of the function
**          _in char* : Name of the DLL which function belong to
** OUTPUT : If succeed return the address of the function otherwise return
**          FALSE.
*/
static inline DWORD             _getFunctionAddress(char *fctname, char *dllname)
 {
     HANDLE                     hDllname;
     DWORD                      addr;

     hDllname = LoadLibraryA((LPCSTR) dllname);
     if (! hDllname)
         return (FALSE);

    addr = (DWORD) GetProcAddress(hDllname, (LPCSTR) fctname);
    if (! addr)
        return (FALSE);

    return (addr);
 }


/*
** DESC   : Get a node by the name.
** INPUT  :
**          _in char* : Name of the function
**          _in char* : Name of the DLL which function belong to
** OUTPUT : If succeed return the node otherwise return FALSE.
*/
static node_t                    *_getNodeByName(char *fctname, char *dllname)
{
    node_t                       *move_s;

    move_s = hookList_gl;
    while (move_s)
    {
        if ( strncmp(move_s->fctname, fctname, strlen(fctname)) == 0 &&
             strncmp(move_s->dllname, dllname, strlen(dllname)) == 0 )
             return (move_s);

        move_s = move_s->next;
    }

    return (NULL);
}

/*
** DESC   : Get a node by the ID.
** INPUT  :
**          _in char* : Name of the function
**          _in char* : Name of the DLL which function belong to
** OUTPUT : If succeed return the node otherwise return FALSE.
*/
static node_t                    *_getNodeById(DWORD id)
{
    node_t                       *move_s;

    move_s = hookList_gl;
    while (move_s)
    {
        if (move_s->nodeid == id)
            return (move_s);
        move_s = move_s->next;
    }
    return (NULL);
}



/****
** PUBLIC API
*/


/*
** DESC   : Get the address which contain the original execution stream.
** INPUT  :
**          _in char* : Name of the function
**          _in char* : Name of the DLL which function belong to
** OUTPUT : If succeed return the re-entrant addresss otherwise return value < 0.
*/
DWORD IHOOKCALL                  getReturnAddressByName(char *fctname, char *dllname)
{
    node_t                       *node;

    node = _getNodeByName(fctname, dllname);
    if (! node)
        return (ERR_NOTLIST);

    return ((DWORD) node->returned);
}


/*
** DESC   : Get the address which contain the original execution stream.
** INPUT  :
**          _in DWORD : Node ID of the hook meta-information
** OUTPUT : If succeed return the re-entrant addresss otherwise return value < 0.
*/
DWORD IHOOKCALL                 getReturnAddressById(DWORD id)
{
    node_t                      *node;

    node = _getNodeById(id);
    if (! node)
        return (ERR_NOTLIST);

    return ((DWORD) node->returned);
}

/*
** DESC   : Remove all the hook data (internal to hookit include) by its ID
** INPUT  :
**          _in DWORD : Node ID of the hook meta-information
** OUTPUT : If succeed return TRUE otherwise return value < 0.
*/
int IHOOKCALL                   unhookById(DWORD id)
{
    node_t                      *move_s;
    node_t                      *prev_s;

    prev_s = 0;
    move_s = hookList_gl;
    while (move_s)
    {
        if (move_s->nodeid == id)
        {
            if (! _unsetHook(move_s->addrHooked, move_s->erased))
                return (ERR_PROT);
            _unsetNode(move_s, prev_s);
            return (TRUE);
        }
        prev_s = move_s;
        move_s = move_s->next;
    }

    return (ERR_NOTLIST);
}

/*
** DESC   : Remove all the hook data (internal to hookit include) by its name
** INPUT  :
**          _in char* : Name of the function
**          _in char* : Name of the DLL which function belong to
** OUTPUT : If succeed return TRUE otherwise return value < 0.
*/
int IHOOKCALL                   unhookByName(char *fctname, char *dllname)
{
    node_t                      *move_s;
    node_t                      *prev_s;

    if (fctname == NULL || dllname == NULL)
        return (ERR_PARAM);

    prev_s = NULL;
    move_s = hookList_gl;
    while (move_s)
    {
        if ( strncmp(move_s->fctname, fctname, strlen(fctname)) == 0 &&
             strncmp(move_s->dllname, dllname, strlen(dllname)) == 0 )
            {
                if (! _unsetHook(move_s->addrHooked, move_s->erased))
                    return (ERR_PROT);
                _unsetNode(move_s, prev_s);
                return (TRUE);
            }
        prev_s = move_s;
        move_s = move_s->next;
    }

    return (ERR_NOTLIST);
}

/*
** DESC   : Hook a windows API function.
** INPUT  :
**          _in char* : Name of the function
**          _in char* : Name of the DLL which function belong to
**          _in DWORD : 32-bit address of the hook function
** OUTPUT : If succeed return a node ID otherwise return value <= 0.
*/
int IHOOKCALL                   hookit(char *fctname, char *dllname, DWORD hookaddr)
{
    DWORD                       addrToHook;
    DWORD                       oldProtect;
    BYTE                       *opcode;
    node_t                      *hooknode_s;
    DWORD                       id;
    int                         lde;

    hooknode_s = (node_t*) malloc(sizeof(node_t));
    if (! hooknode_s)
        return (ERR_ALLOCA);

    oldProtect = 0;
    addrToHook = _getFunctionAddress(fctname, dllname);
    if (! addrToHook)
        return (ERR_ADDR);

    if (! _setPermission(addrToHook, &oldProtect))
        return (ERR_PROT);

    lde = _getLDE(addrToHook);
    if (lde <= 0)
        return (lde);

    opcode = _writeDetour(addrToHook, hookaddr, lde);

    _unsetPermission(addrToHook, oldProtect);

    if (! opcode)
        return (ERR_ALLOCA);

    id = _addHookToList(fctname, dllname, opcode, addrToHook);

    return (id);
}

