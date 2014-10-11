//****************************************************************************
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//  ASFWriter.h
//
//  Declaration of the CASFFileWriter class.
//
//  This is a utility class that the sample uses to create an ASF file by
//  using the Windows Media Format 9.5 SDK.
//
//****************************************************************************

#ifndef __ASFWRITER_H
#define __ASFWRITER_H
#include "stdafx.h"

class CASFFileWriter 
{
public:
    CASFFileWriter();
    HRESULT Init (WCHAR *pwszFileName,
                  WCHAR *pwszProfileName,   
                  WCHAR *pwszCodecString,
                  BITMAPINFOHEADER *pBIH,
                  WAVEFORMATEX *pwfx,
                  DWORD dwVC1Profile,
                  BOOL bIsVBR,
                  double dBitrate, // In bits per second.
                  int msBufferWindow,
                  BYTE* pSeqBuffer,
                  DWORD dwSeqBufferLen,
                  DWORD dwSizeImage);
    HRESULT Destroy();
    HRESULT WriteVideoFrame (BYTE* pVC1Data, DWORD dwVC1DataSize, INT64 qwOutputTimeStamp, FrameType ft);
    HRESULT WriteAudioFrame (BYTE* pAudioData, DWORD dwAudioDataSize, INT64 qwOutputTimeStamp);    

private:
    HRESULT SaveProfile(LPCTSTR ptszFileName, IWMProfile * pIWMProfile);
    HRESULT LoadCustomProfile(LPCTSTR ptszProfileFile, IWMProfile ** ppIWMProfile);
    HRESULT AddVideoStream(DWORD dwVC1Profile, BOOL bIsVBR, double dBitrate,  int msBufferWindow,
                                  BYTE *pBuffer, DWORD dwSizeOfSeq, DWORD dwWidth,
                                  DWORD dwHeight, DWORD dwSizeImage, WCHAR *pwszStreamName);    
    HRESULT SetStreamBasics(IWMStreamConfig * pIWMStreamConfig,
                                  LPWSTR pwszStreamName,
                                  LPWSTR pwszConnectionName,
                                  DWORD dwBitrate,
                                  WM_MEDIA_TYPE * pmt);
    HRESULT CreateEmptyProfile(IWMProfile ** ppIWMProfile);
    HRESULT AddAudioStream(IWMProfile *pWMProfile, DWORD dwMaxRate, 
                           WAVEFORMATEX *pWaveLimits, 
                           WORD *pwStreamNum);

private:
    IWMWriter *m_pWMWriter;
    IWMWriterAdvanced *m_pWMWriterAdvanced;
    IWMProfileManager *m_pWMProfileManager;
    IWMProfile *m_pProfile; 
    WORD m_dwAudioStreamNum;
    WORD m_dwVideoStreamNum;
    WORD m_dwCurrentInputNumber;
    WORD m_dwAudioInputNumber;
    WORD m_dwVideoInputNumber;
    BOOL m_bAudioPresent;
    BOOL m_bVideoPresent;
    WCHAR *m_pwszFileName;
    WCHAR *m_pwszCodecString;
};



#endif
