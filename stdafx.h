//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  stdafx.h
//
//  Contains project-wide includes and definitions.

#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <tchar.h>
#include <vfw.h>
#include <wmsdk.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include "vc1prosdk.h"

using namespace std;

#define FOURCC_WVC1     mmioFOURCC('W','V','C','1')
#define FOURCC_wvc1     mmioFOURCC('w','v','c','1')
#define FOURCC_I420     mmioFOURCC('I','4','2','0') 
#define FOURCC_IYUV     mmioFOURCC('I','Y','U','V') 
#define FOURCC_iyuv     mmioFOURCC('i','y','u','v')
#define FOURCC_i420     mmioFOURCC('i','4','2','0')
#define FOURCC_wmv3     mmioFOURCC('w','m','v','3')
#define FOURCC_WMV3     mmioFOURCC('W','M','V','3')
// Nic
#define FOURCC_YV12     mmioFOURCC('Y','V','1','2') 
#define FOURCC_yv12     mmioFOURCC('y','v','1','2') 

#define DEFINE_GUIDA(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_GUIDA(MEDIASUBTYPE_WVC1, FOURCC_WVC1, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(MEDIASUBTYPE_wvc1,
FOURCC_wvc1, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if (x) { delete x; x = NULL;}
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) if (x) { delete [] x; x = NULL;}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL;}
#endif

#ifndef SAFE_CLOSE_FILEHANDLE
#define SAFE_CLOSE_FILEHANDLE(x) if (x) { CloseHandle(x); x = NULL;}
#endif

#ifndef UNICODE
#define UNICODE
#endif

#endif

void PrintHelp();
