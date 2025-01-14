//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include "../osinclude.h"

#undef  STRICT
#define STRICT
#define VC_EXTRALEAN 1
#include <windows.h>
#include <process.h>
#include <psapi.h>
#include <stdio.h>
#include <stdint.h>

static HANDLE glslang_global_lock;

/* This file contains the Window-OS-specific functions */

#define TO_NATIVE_TLS_INDEX(nIndex) ((DWORD)((uintptr_t)(nIndex) - 1))

/* Thread Local Storage Operations */
OS_TLSIndex OS_AllocTLSIndex(void)
{
    DWORD dwIndex = TlsAlloc();
    if (dwIndex == TLS_OUT_OF_INDEXES)
        return OS_INVALID_TLS_INDEX;
    return (OS_TLSIndex)((uintptr_t)dwIndex + 1);
}

bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
    if (nIndex == OS_INVALID_TLS_INDEX)
        return false;
    if (!TlsSetValue(TO_NATIVE_TLS_INDEX(nIndex), lpvValue))
        return false;
    return true;
}

void *OS_GetTLSValue(OS_TLSIndex nIndex)
{
    return TlsGetValue(TO_NATIVE_TLS_INDEX(nIndex));
}

bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
    if (nIndex == OS_INVALID_TLS_INDEX)
        return false;
    if (!TlsFree(TO_NATIVE_TLS_INDEX(nIndex)))
        return false;
    return true;
}

void InitGlobalLock(void)
{
    glslang_global_lock = CreateMutex(0, false, 0);
}

void GetGlobalLock(void)
{
    WaitForSingleObject(glslang_global_lock, INFINITE);
}

void ReleaseGlobalLock(void)
{
    ReleaseMutex(glslang_global_lock);
}
