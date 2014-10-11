// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// AVI2ASF.cpp : Defines the entry point for the console application.
//
// Copyright (C) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VC1EncASF.h"


int _tmain(int argc, _TCHAR* argv[])
{
    char x = 0;
    DWORD dwError = ERR_OK;

    wprintf (L"\n*************************************\n\n");
    wprintf (L"\nVC-1 AVS2ASF Encoding Sample\n\n");
    wprintf (L"\nCopyright (C) Microsoft Corporation.");
    wprintf (L"\nAll rights reserved.\n\n");
    wprintf (L"\n*************************************\n\n");
	wprintf (L"\nAvisynth, YV12 & Raw support added by Nic.\n\n");
	wprintf (L"\n*************************************\n\n");

    if (argc <= 1 || !wcscmp (L"-?", argv[1]) || !wcscmp (L"-h", argv[1]) || !wcscmp (L"-help", argv[1]))
    {
        PrintHelp();
        return 1;
    }

    /*if (!wcscmp (L"-?", argv[1]))
    {
        PrintHelp();
        return 1;
    }*/       
    
    CVC1EncASF VC1EncASF;

    dwError = VC1EncASF.Init(argc, argv);
   
    clock_t startTime = clock();  

    if(ERR_OK == dwError)
    {
        dwError = VC1EncASF.Analyze(); // 1st pass
    }

    if(ERR_OK == dwError)
    {
        dwError = VC1EncASF.Encode(); // 2nd pass
    }

    VC1EncASF.Destroy();

    if(ERR_OK != dwError)
    {
        wprintf_s(L"\nFinal error code: %d\n", dwError);
    }

    clock_t endTime = clock();
    double duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    wprintf_s (L"Total elapsed time in seconds = %f\n", duration);

    return 0;
}

