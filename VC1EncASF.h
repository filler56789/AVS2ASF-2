//****************************************************************************
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//  VC1EncASF.h
//
//  Declaration of CVC1EncASF class.
//  This is the main class for the AVI2ASF VC-1 sample.
//
//****************************************************************************

#ifndef __VC1ENCASF_H
#define __VC1ENCASF_H
#include "stdafx.h"
#include "AVIReader.h"
#include "ASFWriter.h"

// Supported frame rates.
//const double rgdFramerate[] = {23.976, 24.0, 25.0, 29.97, 30.0, 50.0, 59.94}; 
//const double rgdFRNumerator[] = {24000, 24, 25, 30000, 30, 50, 60000};  
//const double rgdFRDenominator[] = {1001, 1, 1, 1001, 1, 1, 1001};

class CVC1EncASF 
{
public:
    CVC1EncASF(); 
    DWORD Init(int argc, wchar_t *argv[]);
    DWORD Analyze();
    DWORD Encode();
    void Destroy();

private:    
    BOOL get_userConfigProps();
    BOOL get_userAdvancedProps();
    DWORD get_advancedProps();
    DWORD set_advancedProps();
    DWORD ShowStatistics();

    HRESULT InitFileWriter();
    HRESULT InitFileReader();
    void DestroyFileWriter();
    void DestroyFileReader();

    HRESULT WriteAudioVideo(BYTE *pVC1Data, DWORD dwVC1DataSize, INT64 tTimeStampVideo, FrameType ft);
    //BOOL GetFrameRateND(double dFrameRate, double *pdFRNumerator, double *pdFRDenominator);
    DWORD FlushVideo();
    HRESULT FlushAudio();

    ////// Member variables //////    

    // VC-1 Encoder settings.
    COLOR_FORMAT      m_ColorFormat;
    CONFIG_PROPERTIES m_ConfigProps;
    FILTERS           m_Filters;
    ME_PROPERTIES     m_MEProps;
    QUANT_PROPERTIES  m_QuantProps;
    DWORD   m_dwBframeDeltaQP;
    BOOL    m_bClosedEntryPt;
    DWORD   m_dwDisplayHeight;
    DWORD   m_dwDisplayWidth;
    DWORD   m_dwEncodeHeight;
    DWORD   m_dwEncodeWidth;
    BOOL    m_bLBPresent;
    BOOL    m_bLookahead;
    BOOL    m_bAdaptiveGOP;
    DWORD   m_dwKeyPop;
    DWORD   m_dwNumThreads;
    DWORD   m_dwAffinityMask;
    DWORD   m_dwPixHeight;
    DWORD   m_dwPixWidth;
    DWORD   m_dwPixIndex;
    DWORD   m_dwVideoType;
    BOOL    m_bTFF; // top field first flag
    BOOL    m_bRFF; // repeat first field flag
 
    // Class variables.
    CVC1Encoder *m_pEncoder;
    CAVIFileReader  *m_pFileReader;
    CASFFileWriter  *m_pFileWriter;

    // File paths for media.
    WCHAR   m_wszInputFilePath[MAX_PATH];
    WCHAR   m_wszOutputFilePath[MAX_PATH];

    // File path of the .prx file used for generating the audio profile for compression. 
    // See IWMProfileManager::LoadProfileByData for details about using a profile file.
    WCHAR m_wszProfileFilePath [MAX_PATH];

    // Other member variables
    INT64   m_qwOutputSampleTime;
    FrameType m_FrameType;
    BOOL    m_bColorFormat;       // TRUE when user sets COLOR_FORMAT value(s). 
    BOOL    m_bPARI;              // TRUE when user sets PixelAspectRatioIndex.
    double  m_dFRNumerator;
    double  m_dFRDenominator;
    wchar_t **m_ppArgv;           // Pointer to command line string pointer.
    int     m_iArgc;              // Args count.
    DWORD   m_dwOutputBufferSize; // Size of the VC-1 buffer.
    DWORD   m_dwVC1DataSize;      // Size of the data in the buffer.
    BYTE    *m_pVC1Data;          // VC-1 data buffer pointer.
    BITMAPINFOHEADER *m_pBIH;
    WAVEFORMATEX *m_pWFX;
    bool    m_bEncodeRaw;         // Nic
    HANDLE  m_hRawFileHandle;     // Nic
	bool    m_bShowParms;	      // TT
};
#endif
