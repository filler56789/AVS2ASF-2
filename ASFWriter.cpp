//*****************************************************************************
//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//  ASFWriter.cpp
//
//  Implementation of CASFFileWriter.
//
//*****************************************************************************
#include "stdafx.h"
#include "ASFWriter.h"

const LPWSTR wszDefaultConnectionName = L"SDKTest";

CASFFileWriter::CASFFileWriter()
{
    m_pwszFileName  = NULL;
    m_pwszCodecString = NULL;
    m_pWMWriter = NULL;
    m_pWMWriterAdvanced = NULL;
    m_pWMProfileManager = NULL;
    m_pProfile = NULL;    
    m_dwCurrentInputNumber = 0;
    m_dwAudioInputNumber = 0;
    m_dwVideoInputNumber = 0;
    m_dwAudioStreamNum = 1;
    m_dwVideoStreamNum = 2;
    m_bAudioPresent = FALSE; 
    m_bVideoPresent = FALSE; 
}

////////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::Init
//
//  Description: Initializes the file writer class. 
//               Call this method before calling any others.
//
////////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::Init (WCHAR *pwszFileName,
                              WCHAR *pwszProfileName, 
                              WCHAR *pwszCodecString,
                              BITMAPINFOHEADER *pBIH,
                              WAVEFORMATEX *pwfx,
                              DWORD dwVC1Profile,
                              BOOL bIsVBR,
                              double dBitrate,
                              int msBufferWindow,
                              BYTE* pSeqBuffer,
                              DWORD dwSeqBufferLen,
                              DWORD dwSizeImage)
                          
{
    if(NULL == pwszFileName ||
       NULL == pwszCodecString)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;    
    DWORD dwWidth = 0;
    DWORD dwHeight = 0;
    WCHAR *pwszStreamName = 0;
    m_pwszFileName = _wcsdup(pwszFileName);
    m_pwszCodecString = _wcsdup(pwszCodecString);
 
    // It is important that the audio and video input numbers are
    // set in the following order.
    if (NULL != pwszProfileName && (wcslen(pwszProfileName) > 0) && (pwfx!=NULL)) 
    {
        m_bAudioPresent = true; 
        m_dwAudioInputNumber = m_dwCurrentInputNumber++;        
    }
        
    if (NULL != pBIH)
    {
        m_bVideoPresent = true;
        m_dwVideoInputNumber = m_dwCurrentInputNumber++;
        dwWidth = pBIH->biWidth;
        dwHeight = pBIH->biHeight;
    }

    hr = WMCreateWriter(NULL, &m_pWMWriter);

    if(SUCCEEDED(hr))
    {    
        hr = m_pWMWriter->QueryInterface(IID_IWMWriterAdvanced, (void **)&m_pWMWriterAdvanced);
    }

    if(SUCCEEDED(hr))
    {
        hr = WMCreateProfileManager(&m_pWMProfileManager);
    }

    // If there is an audio track, determine whether to use a default
    // profile by searching through codecs or use a custom .prx file,
    // depending on the "-pr" command-line parameter.   
    if(SUCCEEDED(hr))
    {        
        if (TRUE == m_bAudioPresent)
        {
            if (_wcsicmp(pwszProfileName, L"default") == 0)
            {   
                // Test whether a profile exists.
                // If not, create one.
                if (NULL == m_pProfile) 
                {
                    hr = CreateEmptyProfile(&m_pProfile);
                }

                if(NULL == m_pProfile)
                {
                    hr = E_POINTER;
                }

                if(SUCCEEDED(hr))
                {
                   hr = AddAudioStream(m_pProfile, (DWORD)dBitrate, pwfx, &m_dwAudioStreamNum);
                }
            }
            else 
            {
                hr = LoadCustomProfile(pwszProfileName, &m_pProfile);    
            } 
        }        
    }   
 
    if (SUCCEEDED(hr) && 
        TRUE == m_bVideoPresent) 
    {
        hr = AddVideoStream(dwVC1Profile, bIsVBR, dBitrate, msBufferWindow, pSeqBuffer, dwSeqBufferLen,
                            dwWidth, dwHeight, dwSizeImage, pwszStreamName);
    }

#ifdef _DEBUG
    // Save the profile to view for debugging purposes.
    SaveProfile(L"SDKDevAppProfile.prx", m_pProfile);
#endif 

    if(SUCCEEDED(hr))
    {
        hr = m_pWMWriter->SetOutputFilename(m_pwszFileName);
    }

    if(SUCCEEDED(hr))
    {
        // Set the profile on the writer object.
        hr = m_pWMWriter->SetProfile(m_pProfile);
    }

    if(SUCCEEDED(hr) && 
       TRUE == m_bVideoPresent)
    {
        // Set pInput to NULL to signal the SDK that 
        // compressed samples will be sent to the inputs of the writer object.
        hr = m_pWMWriter->SetInputProps(m_dwVideoInputNumber, NULL );
    }
 
    if(SUCCEEDED(hr))
    {
        hr = m_pWMWriter->BeginWriting();
    }

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::Init\nhr = %x \n", hr);
    }

    return hr;
}

////////////////////////////////////////////////////////
//
// CASFFileWriter::Destroy
//
// Description: Frees previously allocated resources.
//
///////////////////////////////////////////////////////

HRESULT CASFFileWriter::Destroy ()
{
    HRESULT hr = S_OK;

    if(m_pWMWriter)
    {       
       hr = m_pWMWriter->EndWriting();
    }        

    SAFE_ARRAY_DELETE(m_pwszFileName);
    SAFE_ARRAY_DELETE(m_pwszCodecString);
    SAFE_RELEASE(m_pWMWriter);
    SAFE_RELEASE(m_pWMWriterAdvanced);
    SAFE_RELEASE(m_pWMProfileManager);
    SAFE_RELEASE(m_pProfile);

    if(m_pwszFileName)
    {
        free(m_pwszFileName);
    }

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::Destroy\nhr = %x \n", hr);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::CreateEmptyProfile
//
//  Description: Creates an empty Windows Media SDK profile 
//               and returns a pointer to the IWMProfile interface.
//
////////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::CreateEmptyProfile(IWMProfile **ppIWMProfile )
{
    if(NULL == ppIWMProfile ||
       NULL == m_pWMProfileManager) 
    {
        return E_POINTER;
    }

    HRESULT hr = m_pWMProfileManager->CreateEmptyProfile( WMT_VER_9_0, ppIWMProfile );

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::CreateEmptyProfile\nhr = %x \n", hr);
    }

    return hr; 
}

//////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::SaveProfile
//
//  Description: Saves the specified profile to disk.
//
/////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::SaveProfile(LPCTSTR ptszFileName, 
                                      IWMProfile *pIWMProfile)
{ 
    if(NULL == ptszFileName || 
       NULL == pIWMProfile  ||
       NULL == m_pWMProfileManager) 
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD dwLength = 0;
    WCHAR *pBuffer = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesWritten = 0;   

  
    // Get the profile length.
    hr = m_pWMProfileManager->SaveProfile(pIWMProfile, NULL, &dwLength);
  
    if(SUCCEEDED(hr))
    {
        // Allocate a buffer for the profile data.
        pBuffer = new WCHAR[dwLength];
        if(NULL == pBuffer) 
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        // Save the profile string.
        hr = m_pWMProfileManager->SaveProfile(pIWMProfile, pBuffer, &dwLength);
    }  

    if(SUCCEEDED(hr))
    { 
        hFile = CreateFile(ptszFileName, 
                            GENERIC_WRITE, 
                            0, 
                            NULL, 
                            CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL);

        if(INVALID_HANDLE_VALUE == hFile) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }        
        else if(FILE_TYPE_DISK != GetFileType(hFile)) 
        {
            hr = NS_E_INVALID_NAME;
        }
        else
        {
            hr = S_OK;
        }
    }   

    if(SUCCEEDED(hr))
    {
        // Write the profile buffer to the file.
        BOOL bResult = WriteFile( hFile, pBuffer, dwLength * sizeof(WCHAR), &dwBytesWritten, NULL);        
        
        if(FALSE == bResult ||
           dwLength * sizeof(WCHAR) != dwBytesWritten) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    SAFE_CLOSE_FILEHANDLE(hFile);
    SAFE_ARRAY_DELETE(pBuffer);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::SaveProfile\nhr = %x \n", hr);
    }

    return hr;
}

/////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::WriteAudioFrame
//
//  Description: Writes the suupplied audio buffer to the ASF file.
//
/////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::WriteAudioFrame (BYTE *pAudioData, DWORD dwAudioDataSize, INT64 qwOutputTimeStamp)
{
    if(NULL == pAudioData)
    {
        return E_POINTER;
    }

    HRESULT  hr = S_OK;
    INSSBuffer *pSample = NULL;
    BYTE *pbBuffer = NULL;
    DWORD cbBuffer = 0;     

    if (FALSE == m_bAudioPresent) 
    { 
        return S_OK;
    }
    
    // Allocate a new sample. This sets the maximum size of the buffer.
    hr = m_pWMWriter->AllocateSample(dwAudioDataSize, &pSample);
   

    if(SUCCEEDED(hr))
    {
        // Get a byte pointer to the buffer.
        hr = pSample->GetBuffer(&pbBuffer);
    }
    
    if( SUCCEEDED(hr))
    {
        // Set the length of the data being added to the buffer.
        // This sets the actual length being used.
        hr = pSample->SetLength(dwAudioDataSize);
    }

    if(SUCCEEDED(hr))
    {
        // Copy the audio data into the buffer.
        memcpy_s(pbBuffer, dwAudioDataSize, pAudioData, dwAudioDataSize);

        // Write the buffer to the output file.
        hr = m_pWMWriter->WriteSample(m_dwAudioInputNumber, qwOutputTimeStamp, 0, pSample);
    }

    SAFE_RELEASE(pSample);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::WriteAudioFrame\nhr = %x \n", hr);
    }

    return hr;
}

////////////////////////////////////////////////////////////
//
//  CASFFileWriter::WriteVideoFrame
//
//  Description: Writes the supplied video buffer to the
//               ASF file.
//
////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::WriteVideoFrame (BYTE* pVC1Data, DWORD dwVC1DataSize, INT64 qwOutputTimeStamp, FrameType ft)
{
    if (FALSE == m_bVideoPresent) 
    { 
        return S_OK;
    }   
  
    HRESULT  hr = S_OK;  
    INSSBuffer* pSample = NULL;
    BYTE* pbBuffer = NULL;
    DWORD dwFlags = 0;

    if (I == ft ||
        II == ft ||
        IP == ft)
    {
        dwFlags = WM_SF_CLEANPOINT;
    }

    // Allocate a new sample. This sets the maximum size of the buffer.
    hr = m_pWMWriter->AllocateSample(dwVC1DataSize, &pSample);

    if(SUCCEEDED(hr))
    {  
        // Get a byte pointer to the buffer.
        hr = pSample->GetBuffer(&pbBuffer);
    }

    if(SUCCEEDED(hr))
    {        
        // Set the length of the data being added to the buffer.
        // This sets the actual length being used.
        hr = pSample->SetLength(dwVC1DataSize);
    }
           
    if(SUCCEEDED(hr))
    {
        // Copy the video data into the buffer.
        memcpy_s(pbBuffer, dwVC1DataSize, pVC1Data, dwVC1DataSize);  

        // Write the buffer to the output file.
        hr = m_pWMWriterAdvanced->WriteStreamSample(m_dwVideoStreamNum, 
                                      qwOutputTimeStamp, 0L, 0L, dwFlags, pSample);            
    }

    SAFE_RELEASE(pSample);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::WriteVideoFrame\nhr = %x \n", hr);
    }
          
    return hr;
}

///////////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::LoadCustomProfile
//
//  Description: 
//
//////////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::LoadCustomProfile( LPCTSTR ptszProfileFile, 
                                            IWMProfile **ppIWMProfile )
{
    if(NULL == ptszProfileFile ||
       NULL == ppIWMProfile ||
       NULL == m_pWMProfileManager) 
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD  dwLength = 0;
    DWORD  dwBytesRead = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR *pwszProfile = NULL; 
    LARGE_INTEGER liLength;
    BOOL bRet = FALSE;

    ZeroMemory(&liLength, sizeof(LARGE_INTEGER));

    // Open the file for reading.
    hFile = CreateFile(ptszProfileFile, 
                       GENERIC_READ, 
                       FILE_SHARE_READ, 
                       NULL, 
                       OPEN_EXISTING, 
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);

    if(INVALID_HANDLE_VALUE == hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if(FILE_TYPE_DISK != GetFileType(hFile))
    {
        hr = NS_E_INVALID_NAME;
    }
    else
    {
        hr = S_OK;
    }

    if(SUCCEEDED(hr))
    {
        // Get the file size.
        bRet = GetFileSizeEx(hFile, &liLength);
        if(FALSE == bRet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());            
        }
    }

    if(SUCCEEDED(hr))
    {
        dwLength = liLength.LowPart;

        // Allocate memory to receive the data from the file.
        pwszProfile = new WCHAR[dwLength + 1];
        if(NULL == pwszProfile)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        // Clear the buffer. It must be null-terminated.
        ZeroMemory(pwszProfile, dwLength +1); 

        // Read the profile into the buffer.
        bRet = ReadFile(hFile, pwszProfile, dwLength, &dwBytesRead, NULL);
        if(FALSE == bRet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if(SUCCEEDED(hr))
    {
        // Load the profile from the buffer.
        hr = m_pWMProfileManager->LoadProfileByData(pwszProfile, ppIWMProfile);     
    }

    SAFE_ARRAY_DELETE(pwszProfile);
    SAFE_CLOSE_FILEHANDLE(hFile);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::LoadCustomProfile\nhr = %x \n", hr);
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::AddVideoStream
// 
//  Description: Adds the VC-1 video stream to the ASF file.
//
////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::AddVideoStream(DWORD dwVC1Profile, BOOL bIsVBR, double dBitrate, int msBufferWindow,
                                        BYTE *pBuffer, DWORD dwSizeOfSeq, DWORD dwWidth, 
                                        DWORD dwHeight, DWORD dwSizeImage, WCHAR *pwszStreamName)
{
    if(NULL == pBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IWMMediaProps *pMediaProps = NULL;    
    IWMStreamConfig *pConfig = NULL;
    IWMPropertyVault *pVault = NULL;
    WM_MEDIA_TYPE  *pMediaType = NULL;
    DWORD  cbMediaType = 0;
    WMVIDEOINFOHEADER *pVIH = NULL;
    
    if (NULL == m_pProfile) 
    {
        hr = CreateEmptyProfile(&m_pProfile);
    }
 
    if(SUCCEEDED(hr))
    {
        // Create the new video stream.
        hr = m_pProfile->CreateNewStream( WMMEDIATYPE_Video, &pConfig );
    }
    
    // Configure the new stream.
    if(SUCCEEDED(hr))
    {
        hr = pConfig->SetStreamNumber(m_dwVideoStreamNum);
    }
    
    if(SUCCEEDED(hr))
    {
        hr = pConfig->SetStreamName(m_pwszCodecString);
    }

    if(SUCCEEDED(hr))
    {
        hr = pConfig->SetConnectionName(wszDefaultConnectionName);
    }

    if(SUCCEEDED(hr))
    {
        // Add the new stream to the profile.
        hr = m_pProfile->AddStream(pConfig);
    }
 
    //Get the media type and modify its properties.
    if(SUCCEEDED(hr))
    {
        hr = pConfig->QueryInterface( IID_IWMMediaProps, (void **)&pMediaProps );
    }

    if(SUCCEEDED(hr))
    {
        hr = pMediaProps->GetMediaType(NULL, &cbMediaType);
    }

    if(SUCCEEDED(hr))
    {
        // Allocate a buffer to receive the media type information.
        pMediaType = (WM_MEDIA_TYPE *) new BYTE[cbMediaType + dwSizeOfSeq];
        if(NULL == pMediaType)
        { 
            hr = E_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        ZeroMemory(pMediaType, cbMediaType + dwSizeOfSeq);

        // Retrieve the media type information.
        hr = pMediaProps->GetMediaType(pMediaType, &cbMediaType);
    }

    if(SUCCEEDED(hr))
    {
        // Set temporal compression.
        pMediaType->bTemporalCompression = TRUE;

        // Get the pointer to the video info header.
        pVIH = (WMVIDEOINFOHEADER *)pMediaType->pbFormat;
     
        // Configure the video info header.
        pVIH->dwBitRate = (DWORD)dBitrate;
        pVIH->rcSource.left = 0;
        pVIH->rcSource.top = 0;
        pVIH->rcSource.right = dwWidth;
        pVIH->rcSource.bottom = dwHeight;
        pVIH->rcTarget.left = 0;
        pVIH->rcTarget.top = 0;
        pVIH->rcTarget.right = dwWidth;
        pVIH->rcTarget.bottom = dwHeight;
        pVIH->bmiHeader.biWidth = dwWidth;
        pVIH->bmiHeader.biHeight = dwHeight;
        pVIH->bmiHeader.biSizeImage = dwSizeImage;

        // Configure members based on profile.
        switch(dwVC1Profile)
        {
            case 0:
            case 1:
                // Simple or Main.
                pVIH->bmiHeader.biCompression = FOURCC_WMV3;
                pMediaType->subtype = WMMEDIASUBTYPE_WMV3;
                break;
            case 2:
                // Advanced
                pVIH->bmiHeader.biCompression = FOURCC_WVC1;
                pMediaType->subtype = MEDIASUBTYPE_WVC1;
                break;
            default:
                // Unexpected.
                hr = E_INVALIDARG;
                break;
        }        

        // Set the format size.
        pMediaType->cbFormat += dwSizeOfSeq;
       
        // Append the VC-1 header information to the end of the WM_MEDIA_TYPE
        // structure.
        memcpy ((BYTE*)pMediaType + cbMediaType , pBuffer, dwSizeOfSeq); 
    }

    if(SUCCEEDED(hr))
    {
        hr = pConfig->SetBitrate((DWORD) dBitrate);
    }
 
    if(SUCCEEDED(hr))
    {
        hr = pConfig->SetBufferWindow(msBufferWindow);
    }

    if(SUCCEEDED(hr))
    {
        hr = pMediaProps->SetMediaType(pMediaType);
    }

    // Set global properties depending on whether the bitrate is VBR or CBR.
    if(SUCCEEDED(hr))
    {       
        hr = pConfig->QueryInterface(IID_IWMPropertyVault, (void**)&pVault);
    } 

    WMT_ATTR_DATATYPE wad = WMT_TYPE_BOOL; // g_wszVBREnabled is boolean.
    DWORD tempDW = 0; 

    if(SUCCEEDED(hr))
    {
        hr = pVault->SetProperty(g_wszVBREnabled, wad, (BYTE*)&bIsVBR, sizeof(WMT_TYPE_BOOL));
    }

    wad = WMT_TYPE_DWORD; // All other properties we set here are DWORD.

    if (bIsVBR)
    {
        // Configure the writer for uncompressed VBR samples.
        if(SUCCEEDED(hr))
        {
            hr = pVault->SetProperty(g_wszVBRBitrateMax, wad, (BYTE*)&tempDW, sizeof(WMT_TYPE_DWORD));
        }

        if(SUCCEEDED(hr))
        {
            hr = pVault->SetProperty(g_wszVBRBufferWindowMax, wad, (BYTE*)&tempDW, sizeof(WMT_TYPE_DWORD));
        }
        
        if(SUCCEEDED(hr))
        {
            hr = pVault->SetProperty(g_wszVBRQuality, wad, (BYTE*)&tempDW, sizeof(WMT_TYPE_DWORD));
        }

        if(SUCCEEDED(hr))
        {
            //The entry L"_RAVG" is for averagebitrate setting for VBR-->VBR profile 
            hr = pVault->SetProperty(L"_RAVG", wad, (BYTE*)&dBitrate, sizeof(WMT_TYPE_DWORD));
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pProfile->SetName(L"ASF Writer");
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pProfile->SetDescription(L"Writes VC-1 compressed video and uncompressed audio into ASF file");
    } 
    // End of setting properties

    if(SUCCEEDED(hr))
    {
        hr = m_pProfile->ReconfigStream(pConfig);
    }

    SAFE_ARRAY_DELETE(pMediaType);
    SAFE_RELEASE(pConfig);
    SAFE_RELEASE(pVault);
    SAFE_RELEASE(pMediaProps);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::AddVideoStream\nhr = %x \n", hr);
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::SetStreamBasics
//
//  Description: Uses the supplied parameters object to
//               set basic properties on the stream config object.
//
/////////////////////////////////////////////////////////////////////
HRESULT CASFFileWriter::SetStreamBasics(IWMStreamConfig *pIWMStreamConfig,
                                          LPWSTR pwszStreamName,
                                          LPWSTR pwszConnectionName,
                                          DWORD dwBitrate,
                                          WM_MEDIA_TYPE *pmt)
{
    if(NULL == pIWMStreamConfig ||
       NULL == pmt)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IWMMediaProps *pMediaProps = NULL;
   
    if(SUCCEEDED(hr))
    {
        hr = pIWMStreamConfig->SetStreamNumber(m_dwAudioStreamNum);
    }

    if(SUCCEEDED(hr))
    {
        hr = pIWMStreamConfig->SetStreamName(pwszStreamName);
    }

    if(SUCCEEDED(hr))
    {
        hr = pIWMStreamConfig->SetConnectionName(pwszConnectionName);
    }

    if(SUCCEEDED(hr))
    {
        hr = pIWMStreamConfig->SetBitrate(dwBitrate);
    }

    if(SUCCEEDED(hr))
    {
        hr = pIWMStreamConfig->QueryInterface(IID_IWMMediaProps, 
                                               (void **)&pMediaProps);
    }

    if(SUCCEEDED(hr))
    {
        hr = pMediaProps->SetMediaType(pmt);
    }  

    SAFE_RELEASE(pMediaProps);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::SetStreamBasics\nhr = %x \n", hr);
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////
//
//  CASFFileWriter::AddAudioStream
//
//  Description: Searches installed codecs to find an audio format that matches 
//               the source audio. Configures and adds the audio stream.
//
//////////////////////////////////////////////////////////////////////////

HRESULT CASFFileWriter::AddAudioStream(IWMProfile *pWMProfile, DWORD dwMaxRate, WAVEFORMATEX *pWaveLimits, WORD *pwStreamNum)
{
    HRESULT hr = S_OK;

    IWMStreamConfig *pConfig = NULL;
    IWMMediaProps* pProps = NULL;
    IWMCodecInfo3 *pCodecInfo = NULL;
    DWORD dwEntries = 0;
    WM_MEDIA_TYPE *pType = NULL;
    DWORD cbType = NULL;
    DWORD dwBestRate = 0;
    DWORD dwPacketsPerSecond = 0;
    const DWORD cdwInvalidIndex = 0xFFFF;
    WAVEFORMATEX *pWave = NULL;

    DWORD dwCodecIndex = cdwInvalidIndex;

    hr = m_pWMProfileManager->QueryInterface(IID_IWMCodecInfo, (void **)&pCodecInfo);

	if(FAILED(hr))
    {
		wprintf_s(L"ERROR: CASFFileWriter::AddAudioStream2 QueryInterface\n");
		wprintf_s(L"       hr = %x\n", hr);
    }

    if(SUCCEEDED(hr))
    {
        hr = pCodecInfo->GetCodecInfoCount(WMMEDIATYPE_Audio, &dwEntries);
    }

    if(SUCCEEDED(hr))
    {
        // Find the index of the codec corresponding to the requested type.
        for(DWORD index = 0; index < dwEntries; index++)
        {
            // Get the first format for each codec. 
            hr = pCodecInfo->GetCodecFormat(WMMEDIATYPE_Audio, index, 0, &pConfig);

            if(SUCCEEDED(hr))
            {  
                // Get the media properties interface.
                hr = pConfig->QueryInterface(IID_IWMMediaProps, (void**)&pProps);
            }

            if(SUCCEEDED(hr))
            {
                // Get the size required for the media type structure.
                hr = pProps->GetMediaType(NULL, &cbType);
            }

            if(SUCCEEDED(hr))
            {
                // Allocate memory for the media type structure.
                pType = (WM_MEDIA_TYPE*) new BYTE[cbType];
                if(pType == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if(SUCCEEDED(hr))
            {
                // Get the media type structure.
                hr = pProps->GetMediaType(pType, &cbType);
            }

            if(SUCCEEDED(hr))
            {
                // Check this codec against the subtype we default to.
                if(pType->subtype == WMMEDIASUBTYPE_WMAudioV8)
                {
                    dwCodecIndex = index;
                }
            }
            
            // The subtypes did not match. Clean up for next iteration.
            SAFE_RELEASE(pConfig);
            SAFE_RELEASE(pProps);
            SAFE_ARRAY_DELETE(pType);

            // Break now if needed. Placing the break here avoids having to
            //  release or delete both inside and outside of the loop.
            if(dwCodecIndex != cdwInvalidIndex ||
               FAILED(hr))
            {
                break;
            }
        } // for index
    }

    if(dwCodecIndex == cdwInvalidIndex)
    {
        hr = E_INVALIDARG;
    }

    if(SUCCEEDED(hr))
    {
        // Get the number of formats supported for the codec.
        hr = pCodecInfo->GetCodecFormatCount(WMMEDIATYPE_Audio, dwCodecIndex, &dwEntries);
    }


    if(SUCCEEDED(hr))
    {
        // Loop through the formats for the codec, looking for matches.
        for(DWORD index = 0; index < dwEntries; index++)
        {
            // Get the next format.
            hr = pCodecInfo->GetCodecFormat(WMMEDIATYPE_Audio, 
                                            dwCodecIndex, 
                                            index, 
                                            &pConfig);

            if(SUCCEEDED(hr))
            {
                // Get the media properties interface.
                hr = pConfig->QueryInterface(IID_IWMMediaProps, (void**)&pProps);
            }

            if(SUCCEEDED(hr))
            {
                // Get the size required for the media type structure.
                hr = pProps->GetMediaType(NULL, &cbType);
            }

            if(SUCCEEDED(hr))
            {
                // Allocate memory for the media type structure.
                pType = (WM_MEDIA_TYPE*) new BYTE[cbType];
                if(pType == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if(SUCCEEDED(hr))
            {
                // Get the media type structure.
                hr = pProps->GetMediaType(pType, &cbType);
            }

            if(SUCCEEDED(hr))
            {
                // Check that the format data is present.
                if(pType->cbFormat >= sizeof(WAVEFORMATEX))
                {
                    pWave = (WAVEFORMATEX*)pType->pbFormat;
                }
                else
                {
                    // The returned media type should always have an attached 
                    //  WAVEFORMATEX structure.  
                    hr = E_UNEXPECTED;
                }

                // Start checking data.

                // Do not check particulars unless the bit rate is in range.
                if((pWave->nAvgBytesPerSec * 8) > dwBestRate &&
                   (pWave->nAvgBytesPerSec * 8) <= dwMaxRate)
                {
                    // Check the limits.
                    if((pWave->nChannels == pWaveLimits->nChannels) &&
                       (pWave->wBitsPerSample == pWaveLimits->wBitsPerSample) &&
                       (pWave->nSamplesPerSec == pWaveLimits->nSamplesPerSec))
                    {                    
                        if((pWave->nAvgBytesPerSec / pWave->nBlockAlign) >= 
                               ((pWave->nAvgBytesPerSec >= 4000) ? 5.0 : 3.0))
                        {
                            // Found a match. We'll use the first good match we find.                          

                            // Set the bit rate.
                            dwBestRate = (pWave->nAvgBytesPerSec * 8);

                            break;

                        }                   
                    } // if matching limits
                } // if valid bit rate
            }

            // Clean up for next iteration.
            SAFE_RELEASE(pConfig);
            SAFE_RELEASE(pProps);
            pWave = NULL;
            SAFE_ARRAY_DELETE(pType);
        } // for index
    }

    if(NULL != pConfig)
    {
        // Found a match.
        hr = SetStreamBasics(pConfig,
                                L"Audio Stream",
                                L"Audio",
                                dwBestRate,
                                pType);
        
    }
    else
    {
        hr = NS_E_AUDIO_CODEC_NOT_INSTALLED;
    }

    if (SUCCEEDED(hr))
    {
        // Add the stream to the profile.
        hr = pWMProfile->AddStream(pConfig);
    }

    SAFE_RELEASE(pCodecInfo);
    SAFE_RELEASE(pConfig);
    SAFE_RELEASE(pProps);
    SAFE_ARRAY_DELETE(pType);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CASFFileWriter::AddAudioStream2\nhr = %x \n", hr);
    }

    return hr;
}

