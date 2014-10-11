//*****************************************************************************
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//  VC1EncASF.cpp
//
//  Implementation of CVC1EncASF.
//
//*****************************************************************************
#include "stdafx.h"
#include "VC1EncASF.h"
#include "Crtdbg.h"

CVC1EncASF::CVC1EncASF()
{
    m_dwBframeDeltaQP = 0;
    m_bClosedEntryPt = FALSE;
    m_dwDisplayHeight = 0;
    m_dwDisplayWidth = 0;
    m_dwEncodeHeight = 0; 
    m_dwEncodeWidth = 0; 
    m_bLBPresent = FALSE;
    m_bLookahead = 0;
    m_bAdaptiveGOP = TRUE;
    m_dwKeyPop = 0;
    m_dwNumThreads = 0; 
    m_dwAffinityMask = 0;
    m_dwPixHeight = 0;
    m_dwPixWidth = 0;
    m_dwPixIndex = 0;
    m_dwVideoType = 0; 
    m_bTFF = FALSE;
    m_bRFF = FALSE;

    m_pVC1Data = NULL; 
    m_dwVC1DataSize = 0;
    m_dwOutputBufferSize = 0;
    m_qwOutputSampleTime = 0;
    m_FrameType = (FrameType)0;
    m_bColorFormat = FALSE;
    m_bPARI = FALSE;
	m_bEncodeRaw = false; // Nic
	m_hRawFileHandle = INVALID_HANDLE_VALUE; // Nic
	m_bShowParms = FALSE; // TT
  
    m_pFileReader = NULL;
    m_pFileWriter = NULL;
    m_pEncoder = NULL;
    m_pBIH = NULL;
    m_pWFX = NULL;
    m_ppArgv = NULL; 
    m_iArgc = 0;

    ZeroMemory(m_wszInputFilePath, MAX_PATH);
    ZeroMemory(m_wszOutputFilePath, MAX_PATH);
    ZeroMemory(m_wszProfileFilePath, MAX_PATH);
    ZeroMemory(&m_ColorFormat, sizeof(COLOR_FORMAT));
    ZeroMemory(&m_ConfigProps, sizeof(CONFIG_PROPERTIES));
    ZeroMemory(&m_Filters, sizeof(FILTERS));
    ZeroMemory(&m_MEProps, sizeof(ME_PROPERTIES));
    ZeroMemory(&m_QuantProps, sizeof(QUANT_PROPERTIES));
}

////////////////////////////////////////////////////////
//
//  CVC1EncASF::Init
//
//  Description: Initialize the encoder SDK and the 
//               encoding sample app.
//
////////////////////////////////////////////////////////
DWORD CVC1EncASF::Init(int argc, wchar_t *argv[])
{    
    DWORD dwError = ERR_OK;
    HRESULT hr = S_OK;

    // Cache the command line args variables.
    m_ppArgv = argv;
    m_iArgc = argc;
 
    // Create the VC-1 encoder class.
    m_pEncoder = new CVC1Encoder;
    if(NULL == m_pEncoder)
    {
        dwError = ERR_MEMORY;
    }

	// Get the user's required settings.
    if(ERR_OK == dwError)
    {
        if(FALSE == get_userConfigProps()) 
        {
			// TT
			wprintf_s(L"\n\nERROR: Error getting user settings\n\n");
            dwError = ERR_INVALIDARG;
        }
    }

    if(ERR_OK == dwError)
    {        
       hr = InitFileReader();
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {  
        hr = m_pFileReader->GetFormat_Video(&m_pBIH);
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {        
       hr = m_pFileReader->GetFormat_Audio(&m_pWFX);
    }

    // Set the width and height based on the AVI reader stats.
    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        m_ConfigProps.dwMaxWidth = m_pBIH->biWidth;
        m_ConfigProps.dwMaxHeight = m_pBIH->biHeight;
    }

	// TT Get framerate
	if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
		hr = m_pFileReader->GetFramerate(&m_ConfigProps.dFrameRate);
		if(m_ConfigProps.dFrameRate < 1.0 || m_ConfigProps.dFrameRate > 2048.0)
		{
			// TT
			wprintf_s(L"\n\nERROR: Input framerate must be between 1 and 2048\n\n");
            dwError = ERR_INVALIDSETTING;
        }
	}

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        // Configure the encoder with the user settings.
        dwError = m_pEncoder->Configure(m_ConfigProps);

		// TT
		if(ERR_OK != dwError)
		{
			wprintf_s(L"\n\nERROR: Error configuring user settings\n\n");
		}
    }

    if(ERR_OK == dwError)
    {
		// Nic
		if ( m_bEncodeRaw )
		{
			dwError = m_pEncoder->SetOutputMode(ES_SH);
			wprintf_s(L"\nINFO: Encoding in ES (Elementry Stream) mode\n\n");
		}
		else
		{
			dwError = m_pEncoder->SetOutputMode(RAW);
			wprintf_s(L"\nINFO: Encoding in ASF mode\n\n");
		}
	}

    // Retrieve the advanced props from the encoder.
    // Because Configure was called,
    // the advanced props should already be set appropriate defaults.
    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        dwError = get_advancedProps();

		// TT
		if(ERR_OK != dwError)
		{
			wprintf_s(L"\n\nERROR: Error getting advanced properties\n\n");
		}
    }

    // Get the user's advanced settings.
    // This will overwrite only the defaults for which
    // the user has specified a value.
    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        if(FALSE == get_userAdvancedProps()) 
        {
			// TT
			wprintf_s(L"\n\nERROR: Error getting user advanced settings\n\n");
            dwError = ERR_INVALIDARG;
        }
    }    

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        // Set the advanced properties.
        dwError = set_advancedProps();

		// TT
		if(ERR_OK != dwError)
		{
			wprintf_s(L"\n\nERROR: Error setting advanced properties\n\n");
		}
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
		// Retrieve the VC-1 data buffer size.
		dwError = m_pEncoder->GetMaxOutputBuffer(&m_dwOutputBufferSize);

		// TT
		if(ERR_OK != dwError)
		{
			wprintf_s(L"\n\nERROR: Error retrieving data buffer size\n\n");
		}
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        if(m_dwOutputBufferSize > 0)
        {
            // Allocate the compressed frame buffer.
            m_pVC1Data = new BYTE[m_dwOutputBufferSize];
            if(NULL == m_pVC1Data)
            {
				// TT
				wprintf_s(L"\n\nERROR: Error allocating output buffer\n\n");
                dwError = ERR_MEMORY;
            }
        }
        else
        {
			// TT
			wprintf_s(L"\n\nERROR: Output buffer size not > 0\n\n");
            dwError = ERR_INVALIDSETTING;
        }
    }

	if (m_bShowParms) {
		wprintf_s(L"\n\n----- Parameters Used: -----\n");

		// TT - print COLOR_FORMAT
		wprintf_s(L"\nCOLOR_FORMAT\n");
		wprintf_s(L"ColorFormatFlag        = %d\n", m_ColorFormat.bColorFormatFlag);
		wprintf_s(L"ColorMatrixCoefficient = %d\n", m_ColorFormat.dwColorMatrixCoefficient);
		wprintf_s(L"ColorPrimaries         = %d\n", m_ColorFormat.dwColorPrimaries);
		wprintf_s(L"ColorTransferChar      = %d\n\n", m_ColorFormat.dwColorTransferChar);

		// TT - print CONFIG_PROPERTIES
		wprintf_s(L"\nCONFIG_PROPERTIES\n");
		wprintf_s(L"Bitrate                = %g\n", m_ConfigProps.dBitRate);
		wprintf_s(L"ComplexityLevel        = %d\n", m_ConfigProps.dwComplexityLevel);
		wprintf_s(L"QP                     = %d\n", m_ConfigProps.dwQP);
		wprintf_s(L"FrameRate              = %g\n", m_ConfigProps.dFrameRate);
		wprintf_s(L"InterlacedSource       = %d\n", m_ConfigProps.bInterlacedSource);
		wprintf_s(L"MaxKeyFrameDistance    = %d\n", m_ConfigProps.dwMaxKeyFrameDistance);
		wprintf_s(L"MaxHeight              = %d\n", m_ConfigProps.dwMaxHeight);
		wprintf_s(L"MaxWidth               = %d\n", m_ConfigProps.dwMaxWidth);
		wprintf_s(L"NumOfBFrames           = %d\n", m_ConfigProps.dwNumOfBFrames);
		wprintf_s(L"PeakBitRate            = %g\n", m_ConfigProps.dPeakBitRate);
		wprintf_s(L"Profile                = %d\n", m_ConfigProps.dwProfile);
		wprintf_s(L"RateControlMode        = %d\n", m_ConfigProps.dwRateControlMode);
		wprintf_s(L"VBVBufferInBytes       = %d\n\n", m_ConfigProps.dwVBVBufferInBytes);

		// TT - print FILTERS
		wprintf_s(L"\nFILTERS\n");
		wprintf_s(L"Denoise                = %d\n", m_Filters.bDenoise);
		wprintf_s(L"InLoop                 = %d\n", m_Filters.bInLoop);
		wprintf_s(L"Median                 = %d\n", m_Filters.bMedian);
		wprintf_s(L"OverlapSmoothing       = %d\n", m_Filters.bOverlapSmoothing);
		wprintf_s(L"NoiseEdgeRemoval       = %d\n\n", m_Filters.bNoiseEdgeRemoval);

		// TT - print ME_PROPERTIES
		wprintf_s(L"\nME_PROPERTIES\n");
		wprintf_s(L"DeltaMVRangeIndex      = %d\n", m_MEProps.dwDeltaMVRangeIndex);
		wprintf_s(L"MotionSearchLevel      = %d\n", m_MEProps.dwMotionSearchLevel);
		wprintf_s(L"MotionSearchMethod     = %d\n", m_MEProps.dwMotionSearchMethod);
		wprintf_s(L"MBModeCost             = %d\n", m_MEProps.dwMBModeCost);
		wprintf_s(L"MVCost                 = %d\n", m_MEProps.dwMVCost);
		wprintf_s(L"MVRange                = %d\n\n", m_MEProps.dwMVRange);

		// TT - print QUANT_PROPERTIES
		wprintf_s(L"\nQUANT_PROPERTIES\n");
		wprintf_s(L"AdaptiveQuant          = %d\n", m_QuantProps.dwAdaptiveQuant);
		wprintf_s(L"DQuantOption           = %d\n", m_QuantProps.dwDQuantOption);
		wprintf_s(L"DQuantPStrength        = %d\n", m_QuantProps.dwDQuantPStrength);
		wprintf_s(L"DQuantBStrength        = %d\n\n", m_QuantProps.dwDQuantBStrength);

		// TT - print other properties
		wprintf_s(L"\nOther Properties\n");
		wprintf_s(L"BframeDeltaQP          = %d\n", m_dwBframeDeltaQP);
		wprintf_s(L"ClosedEntryPt          = %d\n", m_bClosedEntryPt);
		wprintf_s(L"DisplayHeight          = %d\n", m_dwDisplayHeight);
		wprintf_s(L"DisplayWidth           = %d\n", m_dwDisplayWidth);
		wprintf_s(L"EncodeHeight           = %d\n", m_dwEncodeHeight);
		wprintf_s(L"EncodeWidth            = %d\n", m_dwEncodeWidth);
		wprintf_s(L"LBPresent              = %d\n", m_bLBPresent);
		wprintf_s(L"Lookahead              = %d\n", m_bLookahead);
   		wprintf_s(L"AdaptiveGOP            = %d\n", m_bAdaptiveGOP);
		wprintf_s(L"KeyPop                 = %d\n", m_dwKeyPop);
  		wprintf_s(L"NumThreads             = %d\n", m_dwNumThreads);
  		wprintf_s(L"AffinityMask           = %d\n", m_dwAffinityMask);
 		wprintf_s(L"PixHeight              = %d\n", m_dwPixHeight);
  		wprintf_s(L"PixWidth               = %d\n", m_dwPixWidth);
  		wprintf_s(L"PixIndex               = %d\n", m_dwPixIndex);
   		wprintf_s(L"VideoType              = %d\n", m_dwVideoType);
  		wprintf_s(L"TFF                    = %d\n", m_bTFF);
  		wprintf_s(L"RFF                    = %d\n\n\n", m_bRFF);
	}

    if(FAILED(hr) ||
       ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::Init\nhr = %x\nVC1 SDK Error = %d \n", hr, dwError);
    }

    return dwError;
}

/////////////////////////////////////////////////////
//
//  CVC1EncASF::InitFileReader()
//
//  Description: Initialize the AVI file reader class.
//
/////////////////////////////////////////////////////
HRESULT CVC1EncASF::InitFileReader()
{
    HRESULT hr = S_OK;

    //assert(0 != m_dFRNumerator);
    //assert(0 != m_dFRDenominator);
        
    m_pFileReader = new CAVIFileReader();
    if(NULL == m_pFileReader)
    {
        hr = E_OUTOFMEMORY;
    }  
       
    if(SUCCEEDED(hr))
    {
        // TT Pass m_bEncodeRaw to file reader so that audio can be ignored
		hr = m_pFileReader->Init(m_wszInputFilePath, m_bEncodeRaw);
    }
 
    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::InitFileReader\nhr = %x\n", hr);
    }
    return hr;
}

///////////////////////////////////////////////////
//
//  CVC1EncASF::InitFileWriter
//
//  Description: Initialize the ASF file writer class.
//
////////////////////////////////////////////////////
HRESULT CVC1EncASF::InitFileWriter()
{
    if(NULL == m_pEncoder)
    { 
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD dwError = ERR_OK;
    BYTE *pPrivCodecData = NULL;
    DWORD dwPrivCodecDataLen = 0;
    WCHAR *pszCodecString = NULL;
    DWORD dwRateControlMode = 0;
    double dBitRate = 0;
    BOOL bIsVBR = FALSE;  
    int iBufferInMS = 0; // Buffer window in milliseconds. 
  
    // Create an instance of the file writer class.        
    m_pFileWriter = new CASFFileWriter();
    if(NULL == m_pFileWriter)
    {
        hr = E_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr))
    {        
        dwRateControlMode = m_ConfigProps.dwRateControlMode;
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {  
        bIsVBR = (dwRateControlMode == 1 ||
                  dwRateControlMode == 3 ||
                  dwRateControlMode == 4);  

        // ASF writer uses bps, not kbps.
        dBitRate = m_ConfigProps.dBitRate * 1000;
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {     
        // Get the size of the private codec data.
        dwError = m_pEncoder->GetCodecPrivateData(NULL,&dwPrivCodecDataLen, NULL);
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        // Allocate a buffer for the private codec data.
        pPrivCodecData = new BYTE[dwPrivCodecDataLen];
        if (NULL == pPrivCodecData) 
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {   
        ZeroMemory(pPrivCodecData, dwPrivCodecDataLen);

        // Retrieve the private codec data.
        dwError = m_pEncoder->GetCodecPrivateData(pPrivCodecData, &dwPrivCodecDataLen, NULL);
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {  
        // Allocate a buffer for the codec string.
        pszCodecString = new WCHAR[CODECSTR_MAX_LEN];
        if(NULL == pszCodecString)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {   
        // Retrieve the codec string.
        dwError = m_pEncoder->GetCodecString(pszCodecString);
     }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {        
        if (!bIsVBR && m_ConfigProps.dBitRate > 0)
        {
            // Calculate buffer window based on VBV buffer size.
            // dBitRate is in kbps, so the result is in milliseconds.
            iBufferInMS = (int)(m_ConfigProps.dwVBVBufferInBytes * 8 / m_ConfigProps.dBitRate);
        }
        else
        {
            // Let the writer select the buffer window size if VBR.
            iBufferInMS = -1;
        }

        hr = m_pFileWriter->Init(m_wszOutputFilePath, m_wszProfileFilePath, pszCodecString,
              m_pBIH, m_pWFX, m_ConfigProps.dwProfile, bIsVBR, dBitRate,
              iBufferInMS, 
              pPrivCodecData,
              dwPrivCodecDataLen,
              m_dwOutputBufferSize);
    }

    SAFE_ARRAY_DELETE(pPrivCodecData);
    SAFE_ARRAY_DELETE(pszCodecString);

    if(FAILED(hr) ||
       ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"Failure in VC1Enc::InitFileWriter\nhr = %x\nVC1 SDK Error = %d \n", hr, dwError);
    }

    return hr;
}

////////////////////////////////////////////////////////
//
//  CVC1EncASF::get_advancedProps
//
//  Description: Retrieves all the advanced properties.
//
////////////////////////////////////////////////////////
DWORD CVC1EncASF::get_advancedProps()
{
    DWORD dwError = ERR_OK;

    assert(NULL != m_pEncoder);

    // GetColorFormat and GetPixelAspectRatioIndex are deliberately
    // omitted.

    if(m_ConfigProps.dwProfile != 0 && m_ConfigProps.dwNumOfBFrames != 0)
    {
        dwError = m_pEncoder->GetBFrameDeltaQP(&m_dwBframeDeltaQP); 
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetClosedEntryPoint(&m_bClosedEntryPt);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetDisplayResolution(&m_dwDisplayHeight, &m_dwDisplayWidth);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetEncodeResolution(&m_dwEncodeHeight, &m_dwEncodeWidth);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetFilters(&m_Filters);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetLetterBoxPresent(&m_bLBPresent);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetLookAhead(&m_bLookahead);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetAdaptiveGOP(&m_bAdaptiveGOP);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetKeyPopReduction(&m_dwKeyPop);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetMEProperties(&m_MEProps);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetNumOfThreads(&m_dwNumThreads, &m_dwAffinityMask);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetPixelAspectRatio(&m_dwPixHeight, &m_dwPixWidth);
    } 

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetQuantizationProperties(&m_QuantProps);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->GetVideoType(&m_dwVideoType);
    }

    if(ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::get_advancedProps\nerror = %i \n", dwError);
    }

    return dwError;
}

////////////////////////////////////////////////////////
//
//  CVC1EncASF::set_advancedProps
//
//  Description: Specifies all the advanced properties.
//
////////////////////////////////////////////////////////
DWORD CVC1EncASF::set_advancedProps()
{
    DWORD dwError = ERR_OK;

    assert(NULL != m_pEncoder);

	// TT Don't set if no bframes are used
    if(m_ConfigProps.dwProfile != 0 && m_ConfigProps.dwNumOfBFrames != 0)
    {
        dwError = m_pEncoder->SetBFrameDeltaQP(m_dwBframeDeltaQP);
    }

    if(ERR_OK == dwError)
    {
        // Only set color format info if the user provided a value.
        if(TRUE == m_bColorFormat)
        {
            dwError = m_pEncoder->SetColorFormat(m_ColorFormat);
        }
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetClosedEntryPoint(m_bClosedEntryPt);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetDisplayResolution(m_dwDisplayHeight, m_dwDisplayWidth);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetEncodeResolution(m_dwEncodeHeight, m_dwEncodeWidth);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetFilters(m_Filters);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetLetterBoxPresent(m_bLBPresent);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetLookAhead(m_bLookahead);
    }

    if(ERR_OK == dwError)
    {        
        dwError = m_pEncoder->SetAdaptiveGOP(m_bAdaptiveGOP);        
    }

    if(ERR_OK == dwError)
    {        
        dwError = m_pEncoder->SetKeyPopReduction(m_dwKeyPop);        
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetMEProperties(m_MEProps);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetNumOfThreads(m_dwNumThreads, m_dwAffinityMask);
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetPixelAspectRatio(m_dwPixHeight, m_dwPixWidth);                
    }

    if(ERR_OK == dwError)
    {  
        // Set only if the user provided a value.
        if(TRUE == m_bPARI)
        {
            dwError = m_pEncoder->SetPixelAspectRatioIndex(m_dwPixIndex);
        }
    }

    if(ERR_OK == dwError)
    {
        dwError = m_pEncoder->SetQuantizationProperties(m_QuantProps);
    }

    if(ERR_OK == dwError &&
       m_ConfigProps.dwProfile == 2)
    {
        dwError = m_pEncoder->SetVideoType(m_dwVideoType);
    }
  

    if(ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::set_advancedProps\nerror = %i \n", dwError);
    }

    return dwError;
}

///////////////////////////////////////////
//
//  CVC1EncASF::Destroy
//
//  Description: Uninitializes the VC1Enc
//               class and app.
//
///////////////////////////////////////////
void CVC1EncASF::Destroy()
{
    DestroyFileReader();
    SAFE_DELETE(m_pVC1Data);
    SAFE_DELETE(m_pEncoder);

    return;
}

///////////////////////////////////////////
//
//  CVC1EncASF::DestroyFileReader
//
//  Description: Uninitializes the AVI file
//               reader class.
//
///////////////////////////////////////////
void CVC1EncASF::DestroyFileReader()
{
    HRESULT hr = S_OK;

    if (m_pFileReader)
    {
        hr = m_pFileReader->Destroy();
        delete m_pFileReader;
        m_pFileReader = NULL;
    }

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::DestroyFileReader\nhr = %x \n", hr);
    }

    return;
}

///////////////////////////////////////////
//
//  CVC1EncASF::DestroyFileWriter
//
//  Description: Uninitializes the ASF file
//               writer class.
//
///////////////////////////////////////////
void CVC1EncASF::DestroyFileWriter()
{
    HRESULT hr = S_OK;

    if (m_pFileWriter)
    {
        hr = m_pFileWriter->Destroy();
        delete m_pFileWriter;
        m_pFileWriter = NULL;
    }

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::DestroyFileWriter\nhr = %x \n", hr);
    }

    return;
}

//////////////////////////////////////////////////////////////////////
//
//  CVC1EncASF::WriteAudioVideo
//
//  Description: Alternately reads and writes audio and video frames.
//
/////////////////////////////////////////////////////////////////////
HRESULT CVC1EncASF::WriteAudioVideo(BYTE *pVC1Data, DWORD dwVC1DataSize,
                                         INT64 tTimeStampVideo, FrameType ft)
{
    HRESULT hr = S_OK;
    INT64 tTimeStampAudio = 0;
    BYTE *pAudioData = NULL;
    DWORD dwAudioDataSize = 0;

    if(NULL == m_pFileReader ||
       NULL == m_pFileWriter)
    {
        return E_POINTER;
    }
  
    // Get a frame of audio from the reader class.
    hr = m_pFileReader->ReadFrame_Audio(&pAudioData, &dwAudioDataSize, &tTimeStampAudio);

    if (0 == dwAudioDataSize &&
        NULL == pAudioData) 
    {
        // Out of audio data. That's not an error.
        hr = S_OK; 
    }
    else if(SUCCEEDED(hr))
    {  
        hr = m_pFileWriter->WriteAudioFrame(pAudioData, dwAudioDataSize, tTimeStampAudio);
    }       
   
    if (SUCCEEDED(hr) &&
        m_dwVC1DataSize > 0) 
    {
        hr = m_pFileWriter->WriteVideoFrame(pVC1Data, dwVC1DataSize, tTimeStampVideo, ft); 
    }

     if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::WriteAudioVideo\nhr = %x\n", hr);
    }

    return hr;

}

///////////////////////////////////////////////////////////////////
//
//  CVC1EncASF::Analyze
//
//  Description: Performs the 1st encoding pass.
// 
///////////////////////////////////////////////////////////////////
DWORD CVC1EncASF::Analyze()
{
    if(NULL == m_pEncoder ||
       NULL == m_pFileReader)
    {
        return ERR_POINTER;
    }

    // Don't analyze for 1-pass modes.
    if(m_ConfigProps.dwRateControlMode < 2)
    {
        return ERR_OK;
    }

    HRESULT hr = S_OK;
    DWORD dwError = ERR_OK;
    DWORD dwNumBytesRead = 0;
    BYTE *pVideoData = NULL;
    DWORD dwVideoDataSize = 0;
    INT32 iTotalFrames =  0;
	int iCurrPercent =  0;
	int iPrevPercent =  101;

    // Get frame count.       
    hr =  m_pFileReader->GetFrameCount(&iTotalFrames);

    if(SUCCEEDED(hr))
    {
        m_pFileReader->SeekFirst();

        // Start.  
        dwError = m_pEncoder->StartAnalyze();

        wprintf (L"\n------------Start Analysis---------\n");
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        // Iterate frame-by-frame.
        for (INT32 iFrame = 0; iFrame < iTotalFrames; iFrame++)
        {
            INT64 tTimeStampIn = 0;
               
            hr = m_pFileReader->ReadFrame_Video(&pVideoData, &dwVideoDataSize, &tTimeStampIn);          

            if(SUCCEEDED(hr))
            {
				// TT Do percent completed
				iCurrPercent = int(100.0 * float(iFrame) / float(iTotalFrames));

					if (iCurrPercent != iPrevPercent)
					{
						wprintf(L"------------Analyzing %02d%%----------\r", iCurrPercent);
						iPrevPercent = iCurrPercent;
					}

                dwError = m_pEncoder->AnalyzeFrame (pVideoData);

                if (ERR_OK != dwError)
                {
                    break;
                }
            }
            else
            {
                // ReadFrame_Video failed for some reason.
                break;
            }
        }
    }

    if(ERR_OK == dwError &&
       SUCCEEDED(hr))
    { 
        // Wrap up.   
        dwError = m_pEncoder->EndAnalyze();

        wprintf (L"------------End Analysis-----------\n\n"); 
    }

    if(ERR_OK == dwError &&
       SUCCEEDED(hr))
    { 
        // Display the statistics.
        dwError = ShowStatistics();
    }

    if(FAILED(hr) ||
       ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"\nFailure in Analyze\nhr = %x\nVC1 SDK Error = %i \n", hr, dwError);
    }   

    return dwError; 
}

///////////////////////////////////////////////////////////////////
//
//  CVC1EncASF::Encode
//
//  Description: Performs the 2nd encoding pass.
// 
///////////////////////////////////////////////////////////////////
DWORD CVC1EncASF::Encode()
{
    if(NULL == m_pEncoder ||
       NULL == m_pFileReader)
    {
        return ERR_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD dwError = ERR_OK;
    DWORD dwNumBytesRead = 0;
    BYTE *pVideoData = NULL;
    DWORD dwVideoDataSize = 0;
    INT64 tTimeStampOut = 0;
    INT32 iTotalFrames =  0;
	int iCurrPercent =  0;
	int iPrevPercent =  101;

    // Get frame count.       
    hr =  m_pFileReader->GetFrameCount(&iTotalFrames);

    if(SUCCEEDED(hr))
    {
        m_pFileReader->SeekFirst();

        // Start.
        dwError = m_pEncoder->StartEncode();

        wprintf (L"\n------------Start Encode-----------\n");         
    }

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
  		// Nic
		if ( m_bEncodeRaw )
		{
			m_hRawFileHandle = CreateFile(m_wszOutputFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
			if ( m_hRawFileHandle == INVALID_HANDLE_VALUE )
			{
				wprintf (L"ERROR: Failed to open %s for writing!\n", m_wszOutputFilePath);
				hr = E_FAIL;
			}
		}
		else
		{
			// Initialize the ASF writer only for the encoding phase.
			hr = InitFileWriter();
		}
    } 

    if(SUCCEEDED(hr) &&
       ERR_OK == dwError)
    {
        // Iterate frame-by-frame.
        for (INT32 iFrame = 0; iFrame < iTotalFrames; iFrame++)
        {
            INT64 tTimeStampIn = 0;
               
            hr = m_pFileReader->ReadFrame_Video(&pVideoData, &dwVideoDataSize, &tTimeStampIn);          

            if(SUCCEEDED(hr))
            {
				// TT Do percent completed
				iCurrPercent = int(100.0 * float(iFrame) / float(iTotalFrames));

				if (iCurrPercent != iPrevPercent)
				{
					wprintf(L"------------Processing %02d%%---------\r", iCurrPercent);
					iPrevPercent = iCurrPercent;
				}
            }
            else
            {
                // ReadFrame_Video failed.
                break;
            }

            if(SUCCEEDED(hr))
            {
                ZeroMemory(m_pVC1Data, m_dwOutputBufferSize);
                dwError = m_pEncoder->EncodeFrame (pVideoData, m_pVC1Data, &m_dwVC1DataSize, tTimeStampIn, 
                                           &tTimeStampOut, m_bTFF, m_bRFF, &m_FrameType);
            
                if(ERR_OK == dwError)
                {
                    // Nic
					if ( m_bEncodeRaw )
					{
						DWORD dwBytesWritten = 0;
						if ( !WriteFile(m_hRawFileHandle, m_pVC1Data, m_dwVC1DataSize, &dwBytesWritten, NULL) )
						{
							wprintf (L"ERROR: Failed to write raw file! Exiting.\n");
							hr = E_FAIL;
							break;
						}
					}
					else
						dwError = WriteAudioVideo(m_pVC1Data, m_dwVC1DataSize, tTimeStampOut, m_FrameType);
                }
                else if(dwError == ERR_NO_OP_FRAME)
                {
                    // Don't write for no-op frames, just move on.
                    continue;
                }
                else
                {
                    // Encode frame error.
                    break;
                }
            }
        } // for iFrame
    }

    if(ERR_OK == dwError &&
       SUCCEEDED(hr)) 
    {   
        dwError = FlushVideo();        
    }

    if (SUCCEEDED(hr) &&
        ERR_OK == dwError)        
    {
        hr = FlushAudio();        
    }

    if(ERR_OK == dwError &&
       SUCCEEDED(hr))
    { 
        // Wrap up.     
        dwError = m_pEncoder->EndEncode();

        wprintf (L"------------End Encode-------------\n\n");
        wprintf_s (L"------------Done-------------------\n\n");
    }

    if(ERR_OK == dwError &&
       SUCCEEDED(hr))
    {
        // Display the statistics.
        dwError = ShowStatistics();
    }        
	
	// Nic
	if ( m_hRawFileHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hRawFileHandle);
		m_hRawFileHandle = INVALID_HANDLE_VALUE;
	}
	else

	// TT Close ASF file writer unless raw output
	if (!m_bEncodeRaw)
	{
		DestroyFileWriter();
	}

    if(FAILED(hr) ||
       ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"\nFailure in Encode\nhr = %x\nVC1 SDK Error = %i \n", hr, dwError);
    }   

    return dwError;   
}

///////////////////////////////////////////////////////////////////
//
//  CVC1EncASF::FlushVideo
//
//  Description: Flush the video encoding queue.
// 
///////////////////////////////////////////////////////////////////
DWORD CVC1EncASF::FlushVideo()
{
    DWORD dwError = ERR_OK;
    INT64 tTimeStampOut = 0;

    // Flush any remaining frames.
    while(ERR_NOMORE_FRAMES != dwError)
    {
        ZeroMemory(m_pVC1Data, m_dwVC1DataSize);

        dwError = m_pEncoder->Flush(m_pVC1Data, &m_dwVC1DataSize, &tTimeStampOut, &m_FrameType);

        if((ERR_OK == dwError || ERR_NOMORE_FRAMES == dwError)
			&& (m_dwVC1DataSize > 0))
        {
			// Nic
			if ( m_bEncodeRaw )
			{
				DWORD dwBytesWritten = 0;
				if ( !WriteFile(m_hRawFileHandle, m_pVC1Data, m_dwVC1DataSize, &dwBytesWritten, NULL) )
				{
					wprintf_s(L"ERROR: Failed to flush RAW file!\n");
					break;
				}
			}
			else
			{
				  if (ERR_IO == WriteAudioVideo(m_pVC1Data, m_dwVC1DataSize, tTimeStampOut, m_FrameType))
				  {
					break;
				  }
			}
        }
    } 

    if(ERR_NOMORE_FRAMES == dwError)
    {
        dwError = ERR_OK;
    }

     if(ERR_OK != dwError)
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::FlushVideo\nVC1 SDK Error = %d \n", dwError);
    }

    return dwError;
}

///////////////////////////////////////////////////////////////////
//
//  CVC1EncASF::FlushAudio
//
//  Description: Writes remaining audio data.
// 
///////////////////////////////////////////////////////////////////
HRESULT CVC1EncASF::FlushAudio()
{
    HRESULT hr = S_OK;
    BYTE *pAudioData = NULL;
    DWORD dwAudioDataSize = 0;
    INT64 tTimeStampAudio = 0;

	// Nic
    if(NULL == m_pFileReader || ( NULL == m_pFileWriter && m_bEncodeRaw == false ) )
    {
        return ERR_POINTER;
    }

    // Flush audio.
    do 
    {
        hr = m_pFileReader->ReadFrame_Audio(&pAudioData, &dwAudioDataSize, &tTimeStampAudio);

        if(SUCCEEDED(hr) &&
           dwAudioDataSize > 0) 
        { 
            hr = m_pFileWriter->WriteAudioFrame(pAudioData, dwAudioDataSize, tTimeStampAudio);            
        }         
    }while(dwAudioDataSize > 0);

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CVC1EncASF::FlushAudio\nhr = %x\n", hr);
    }

    return hr;
}

///////////////////////////////////////////////////////////////
//
//  CVC1EncASF::get_userConfigProps
//
//  Description: Gets configuration values from the command line.
// 
///////////////////////////////////////////////////////////////
BOOL CVC1EncASF::get_userConfigProps()
{
#ifdef _DEBUG
    // Some of the secure string functions, such as _wcslwr_s,
    // pad the buffer with a fixed character during debugging. The following
    // call shuts this off for argument parsing because this padding overwrites
    // parts of the buffer we still want to use.
    size_t sizeOld = _CrtSetDebugFillThreshold(0);
#endif

    // Set up some convenient defaults for purposes of the sample.
	m_ConfigProps.dBitRate = 0;
	m_ConfigProps.dwComplexityLevel = 0;
	m_ConfigProps.bInterlacedSource = FALSE;
	m_ConfigProps.dwMaxKeyFrameDistance = 25;
	m_ConfigProps.dwNumOfBFrames = 1;
	m_ConfigProps.dPeakBitRate = 0;
	m_ConfigProps.dwProfile = 2;
	m_ConfigProps.dwRateControlMode = 1;
	m_ConfigProps.dwVBVBufferInBytes = 0;
    wcscpy_s(m_wszProfileFilePath, MAX_PATH, L"default"); // If no .prx specified, find a matching codec.

    // Required settings flags.
    BOOL bInfilename = FALSE;
    BOOL bOutfilename = FALSE;

    WCHAR *wszStopString = NULL;   

    // Get the required arguments.
    for (int i = 1; i < m_iArgc; i++)
    {  
        if(sizeof(m_ppArgv[i]) > MAX_PATH)
        {
            wprintf_s (L"\nERROR: Argument %s too long.\n", m_ppArgv[i]);
            PrintHelp();

            return FALSE;
        }

        // Convert text to lowercase.
        _wcslwr_s(m_ppArgv[i], MAX_PATH);            

        //////////////////////// Required arguments //////////////////////////////

        if (!wcscmp (L"-i", m_ppArgv[i]))
        {
            // Input file.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            bInfilename = TRUE;
            wcscpy_s(m_wszInputFilePath, MAX_PATH, m_ppArgv[i]);

            size_t iFileLen = wcslen(m_wszInputFilePath);

			// Nic
            if (_wcsicmp(&m_wszInputFilePath[iFileLen-3], L"avi") != 0 && 
				_wcsicmp(&m_wszInputFilePath[iFileLen-3], L"avs") != 0)            
			{
                // We don't need to fail, but we should warn the user that this is 
                // unexpected.
                wprintf_s (L"\nWARNING: Unexpected file name extension for input file.\n");
                wprintf_s (L"\tExpected either .avi or .avs.\n\n");
            }
        }

        else if (!wcscmp (L"-o", m_ppArgv[i]) )
        {
            // Output file.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            bOutfilename = TRUE;
            wcscpy_s(m_wszOutputFilePath, MAX_PATH, m_ppArgv[i]);

            size_t iFileLen = wcslen(m_wszOutputFilePath);

			// Nic
			if ( _wcsicmp(&m_wszOutputFilePath[iFileLen-3], L"vc1") == 0 || 
				_wcsicmp(&m_wszOutputFilePath[iFileLen-3], L"raw") == 0 )
			{
				m_bEncodeRaw = true;
			}
			else
			{
				if (_wcsicmp(&m_wszOutputFilePath[iFileLen-3], L"asf") != 0 &&
					_wcsicmp(&m_wszOutputFilePath[iFileLen-3], L"wmv") != 0)
				{
					// We don't need to fail, but we should warn the user that this is
					// unexpected.
					wprintf_s (L"\nWARNING: Unexpected file name extension for output file.\n");
					wprintf_s (L"\tExpected .asf or .wmv.\n\n");
				}
			}
        }

        ///////////////////////// CONFIG_PROPERTIES ////////////////////////////

        // Note that max width, max height and framerate are retrieved from the AVI file properties.
        // Note that some of these settings are required on the command line.

        else if (!wcscmp (L"-rate", m_ppArgv[i]))
        {
            // Bit rate.
            // Required.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dBitRate = wcstod(m_ppArgv[i], &wszStopString);

        }

        else if (!wcscmp (L"-complexity", m_ppArgv[i]))
        {
            // Encoder complexity.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwComplexityLevel = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_ConfigProps.dwComplexityLevel > 5)
            {
                wprintf_s (L"\nERROR: Invalid complexity: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 5);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-qp", m_ppArgv[i]))
        {
            // Fixed or max QP.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwQP = wcstoul(m_ppArgv[i], NULL, 10);

			// TT Check max QP
			if (m_ConfigProps.dwQP > 31)
            {
                wprintf_s (L"\nERROR: QP cannot be > 31.\n", m_ppArgv [i]);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-interlaced", m_ppArgv[i]))
        {
            // Interlaced source video.
            m_ConfigProps.bInterlacedSource = TRUE;       
        }

        else if (!wcscmp (L"-maxkeydist", m_ppArgv[i]))
        {
            // Max key frame distance. 
            // Required.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwMaxKeyFrameDistance =  wcstoul(m_ppArgv[i], NULL, 10); 

        }

        else if (!wcscmp (L"-bframes", m_ppArgv[i]))
        {
            // Number of B frames
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwNumOfBFrames = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_ConfigProps.dwNumOfBFrames > 7)
            {
                wprintf_s (L"\nERROR: Invalid number of B-frames %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 7);
                return FALSE;
            }

        }

        else if (!wcscmp (L"-peakrate", m_ppArgv[i]))
        {
            // Peak bit rate.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dPeakBitRate = wcstod(m_ppArgv[i], &wszStopString);

            if (m_ConfigProps.dPeakBitRate < 0.0)
            {
                wprintf_s (L"\nERROR: Invalid datarate %s.\n", m_ppArgv [i]);
                return FALSE;
            }

        }

        else if (!wcscmp (L"-profiletype", m_ppArgv[i]))
        {
            // Simple, Main, Advanced profile
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwProfile = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_ConfigProps.dwProfile > 2)
            {
                wprintf_s (L"\nERROR: Invalid profile. %s.  0 = Simple 1= Main 2 = Advanced\n", m_ppArgv [i]);
                return FALSE;
            }

        }

        else if (!wcscmp (L"-ratecontrol", m_ppArgv[i]))
        {
            // rate control mode
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwRateControlMode  = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_ConfigProps.dwRateControlMode  > 4)
            {
                wprintf_s (L"\nERROR: Invalid rate control mode. %s. \n0 = 1-pass CBR\n1 = 1-pass VBR fixed QP\n", m_ppArgv [i]);
                wprintf_s (L"2 = 2-pass CBR\n3 = 2-pass peak constrained VBR\n4 = 2-pass unconstrained VBR.");
                return FALSE;
            }
        }

        else if (!wcscmp (L"-vbv", m_ppArgv[i]))
        {
            // VBV buffer size, in bytes.
            // Required.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_ConfigProps.dwVBVBufferInBytes  = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_ConfigProps.dwVBVBufferInBytes  > 4294967295)
            {
                wprintf_s (L"\nERROR: VBV Buffer too large %s. %d.\n", m_ppArgv [i], 4294967295);
                return FALSE;
            }

        }

        // Not looking for these on this pass, but we need to deal with them gracefully.
 
        // These take no arguments, but are legitimate commands.
        else if (!wcscmp (L"-lbpresent", m_ppArgv[i]) ||
                 !wcscmp (L"-lookahead", m_ppArgv[i]) ||
                 !wcscmp (L"-adaptivegop", m_ppArgv[i]) ||                 
                 !wcscmp (L"-tff", m_ppArgv[i]) ||
				 !wcscmp (L"-showparms", m_ppArgv[i]) || // TT
                 !wcscmp (L"-rff", m_ppArgv[i]))
        {
            // Just ignore these.
        }

        // These take 1 argument. We'll need to skip over the argument.
        else if (!wcscmp (L"-bdeltaqp", m_ppArgv[i]) ||
                 !wcscmp (L"-denoise", m_ppArgv[i]) ||
                 !wcscmp (L"-inloop", m_ppArgv[i]) ||
                 !wcscmp (L"-median", m_ppArgv[i]) ||
                 !wcscmp (L"-overlap", m_ppArgv[i])||
                 !wcscmp (L"-noiseedge", m_ppArgv[i]) ||
                 !wcscmp (L"-deltamvrange", m_ppArgv[i]) ||
                 !wcscmp (L"-motionsearchlevel", m_ppArgv[i]) ||
                 !wcscmp (L"-mesearchmethod", m_ppArgv[i])  ||
                 !wcscmp (L"-mbcost", m_ppArgv[i]) ||
                 !wcscmp (L"-mvcost", m_ppArgv[i]) ||
                 !wcscmp (L"-mvrange", m_ppArgv[i]) ||
                 !wcscmp (L"-adaptivequant", m_ppArgv[i]) ||
                 !wcscmp (L"-dquantoption", m_ppArgv[i]) ||
                 !wcscmp (L"-dquantpstrength", m_ppArgv[i]) ||
				 !wcscmp (L"-dquantbstrength", m_ppArgv[i]) ||
                 !wcscmp (L"-closedentrypt", m_ppArgv[i]) ||
				 !wcscmp (L"-sarx", m_ppArgv[i]) ||
				 !wcscmp (L"-sary", m_ppArgv[i]) ||
                 !wcscmp (L"-dw", m_ppArgv[i]) ||
                 !wcscmp (L"-dh", m_ppArgv[i]) ||
                 !wcscmp (L"-ew", m_ppArgv[i]) ||
                 !wcscmp (L"-eh", m_ppArgv[i]) ||                 
                 !wcscmp (L"-threads", m_ppArgv[i]) ||
                 !wcscmp (L"-affinity", m_ppArgv[i]) ||
                 !wcscmp (L"-videotype", m_ppArgv[i]) ||
                 !wcscmp (L"-keypop", m_ppArgv[i]) ||
                 !wcscmp (L"-pr", m_ppArgv[i]))
        {
            i++;
        }

        // This one might take 3 arguments.
        else if (!wcscmp (L"-colorformatflags", m_ppArgv[i]))
        {
            // Color primary.
            i++;

            // Convert the argument to a Boolean.
            BOOL btempflag = (BOOL) _wtoi(m_ppArgv[i]);

            if (btempflag) 
            {
                i += 3;
            }
        }

        // This one might take 2 arguments.
        else if (!wcscmp (L"-aspectratio", m_ppArgv[i]))
        {
            // Pixel aspect ratio.
            i++;

            // Convert the argument to a number.
            DWORD dwPixIndex = wcstoul(m_ppArgv[i], NULL, 10);

            // 15 means "Aspect width and height transmitted"
            // This means there will be two more arguments
            if (dwPixIndex == 15)
            {
                i += 2;                  
            }           
        }

        else
        {
            // Unknown option.
            wprintf_s (L"\nERROR: Unknown option %s.\n", m_ppArgv[i]);
      
            //PrintHelp();
            return FALSE;
        }
    }

#ifdef _DEBUG
    _CrtSetDebugFillThreshold(sizeOld);
#endif

    //Check that we have the minimum parameters needed to encode
    if(!(bInfilename && bOutfilename))
    {    
        PrintHelp();
		wprintf_s (L"\nERROR: You must provide input and output names.\n");
        return FALSE;
    }

	// TT Validate options against rate control and reset values if needed
	wprintf_s (L"\nINFO: Validate CONFIG_PROPERTIES\n");

	if (m_ConfigProps.dwMaxKeyFrameDistance <= m_ConfigProps.dwNumOfBFrames)
	{
		wprintf_s (L"ERROR: Max keyframe distance must be > Number of B frames.\n");
		return FALSE;
	}

	if (m_ConfigProps.dwRateControlMode == 0)
	{
		if (m_ConfigProps.dwQP == 0)
		{
			wprintf_s (L"INFO: QP must be > 0 for Rate Control 0 - setting to 31\n");
			m_ConfigProps.dwQP = 31;
		}
		if (m_ConfigProps.dBitRate == 0)
		{
			wprintf_s (L"ERROR: Bitrate must be set for Rate Control 0\n");
			return FALSE;
		}
		if (m_ConfigProps.dwVBVBufferInBytes == 0)
		{
			wprintf_s (L"ERROR: VBV buffer must be set for Rate Control 0\n");
			return FALSE;
		}
		if (m_ConfigProps.dPeakBitRate != 0)
		{
			wprintf_s (L"INFO: Peak bitrate set to 0 for Rate Control 0\n");
			m_ConfigProps.dPeakBitRate = 0;
		}
	}
	else if (m_ConfigProps.dwRateControlMode == 1)
	{
		if (m_ConfigProps.dBitRate != 0)
		{
			wprintf_s (L"INFO: Bitrate set to 0 for Rate Control 1\n");
			m_ConfigProps.dBitRate = 0;
		}
		if (m_ConfigProps.dPeakBitRate != 0)
		{
			wprintf_s (L"INFO: Peak bitrate set to 0 for Rate Control 1\n");
			m_ConfigProps.dPeakBitRate = 0;
		}
		if (m_ConfigProps.dwVBVBufferInBytes != 0)
		{
			wprintf_s (L"INFO: VBV buffer set to 0 for Rate Control 1\n");
			m_ConfigProps.dwVBVBufferInBytes = 0;
		}
		if (m_ConfigProps.dwQP == 0)
		{
			wprintf_s (L"INFO: QP must be > 0 for Rate Control 1 - setting to 3\n");
			m_ConfigProps.dwQP = 3;
		}
	}
	else if (m_ConfigProps.dwRateControlMode == 2)
	{
		if (m_ConfigProps.dwQP == 0)
		{
			wprintf_s (L"INFO: QP must be > 0 for Rate Control 2 - setting to 31\n");
			m_ConfigProps.dwQP = 31;
		}
		if (m_ConfigProps.dBitRate == 0)
		{
			wprintf_s (L"ERROR: Bitrate must be set for Rate Control 2\n");
			return FALSE;
		}
		if (m_ConfigProps.dwVBVBufferInBytes == 0)
		{
			wprintf_s (L"ERROR: VBV buffer must be set for Rate Control 2\n");
			return FALSE;
		}
		if (m_ConfigProps.dPeakBitRate != 0)
		{
			wprintf_s (L"INFO: Peak bitrate set to 0 for Rate Control 2\n");
			m_ConfigProps.dPeakBitRate = 0;
		}
	}
	else if (m_ConfigProps.dwRateControlMode == 3)
	{
		if (m_ConfigProps.dwQP != 0)
		{
			wprintf_s (L"INFO: QP set to 0 for Rate Control 3\n");
			m_ConfigProps.dwQP = 0;
		}
		if (m_ConfigProps.dBitRate == 0)
		{
			wprintf_s (L"ERROR: Bitrate must be set for Rate Control 3\n");
			return FALSE;
		}
		if (m_ConfigProps.dwVBVBufferInBytes == 0)
		{
			wprintf_s (L"ERROR: VBV buffer must be set for Rate Control 3\n");
			return FALSE;
		}
		if (m_ConfigProps.dPeakBitRate == 0)
		{
			wprintf_s (L"ERROR: Peak Rate must be set for Rate Control 3\n");
			return FALSE;
		}
	}
	else if (m_ConfigProps.dwRateControlMode == 4)
	{
		if (m_ConfigProps.dwQP != 0)
		{
			wprintf_s (L"INFO: QP set to 0 for Rate Control 4\n");
			m_ConfigProps.dwQP = 0;
		}
		if (m_ConfigProps.dPeakBitRate != 0)
		{
			wprintf_s (L"INFO: Peak bitrate set to 0 for Rate Control 4\n");
			m_ConfigProps.dPeakBitRate = 0;
		}
		if (m_ConfigProps.dwVBVBufferInBytes != 0)
		{
			wprintf_s (L"INFO: VBV buffer set to 0 for Rate Control 4\n");
			m_ConfigProps.dwVBVBufferInBytes = 0;
		}
	}

	wprintf_s (L"INFO: Validate CONFIG_PROPERTIES - done\n\n");

    return TRUE;
}

///////////////////////////////////////////////////////////////
//
//  CVC1EncASF::get_userAdvancedProps
//
//  Description: Gets advanced properties from the command line.
// 
///////////////////////////////////////////////////////////////
BOOL CVC1EncASF::get_userAdvancedProps()
{
#ifdef _DEBUG
    // Some of the secure string functions, such as _wcslwr_s,
    // pad the buffer with a fixed character during debugging. The following
    // call shuts this off for argument parsing because this padding overwrites
    // parts of the buffer we still want to use.
    size_t sizeOld = _CrtSetDebugFillThreshold(0);
#endif

    WCHAR *wszStopString = NULL;
	DWORD dwSARx = 0;
	DWORD dwSARy = 0;

    for (int i = 1; i < m_iArgc; i++)
    {
        if(sizeof(m_ppArgv[i]) > MAX_PATH)
        {
            wprintf_s (L"\nERROR: Argument %s too long.\n", m_ppArgv[i]);
            PrintHelp();

            return FALSE;
        }

        // Convert text to lowercase.
        _wcslwr_s(m_ppArgv[i], MAX_PATH);

         ////////////////// COLOR_FORMAT ///////////////////////

        if (!wcscmp (L"-colorformatflags", m_ppArgv[i]))
        {
            // Color primary.
            i++;

            m_bColorFormat = TRUE;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a Boolean.
            m_ColorFormat.bColorFormatFlag = (BOOL) _wtoi(m_ppArgv[i]);

            if (m_ColorFormat.bColorFormatFlag) 
            {
                i++;

                if (i == m_iArgc)
                {
                    wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                    return FALSE;
                }
               
                // Convert the argument to a number.
                m_ColorFormat.dwColorPrimaries = wcstoul(m_ppArgv[i], NULL, 10);
                
                i++;

                if (i == m_iArgc)
                {
                    wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                    return FALSE;
                }

                // Convert the argument to a number.
                m_ColorFormat.dwColorTransferChar = wcstoul(m_ppArgv[i], NULL, 10);

                i++;

                if (i == m_iArgc) 
                {
                    wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                    return FALSE;
                }

                // Convert the argument to a number.
                m_ColorFormat.dwColorMatrixCoefficient = wcstoul(m_ppArgv[i], NULL, 10);
            }
        }

        /////////////////// FILTERS /////////////////////////

        else if (!wcscmp (L"-denoise", m_ppArgv[i]))
        {
            //Denoise pre-processing filter on or off.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_Filters.bDenoise = (BOOL)_wtoi(m_ppArgv[i]);
        }

        else if (!wcscmp (L"-inloop", m_ppArgv[i])) 
        {
            // In-loop filter on or off.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_Filters.bInLoop = (BOOL)_wtoi(m_ppArgv[i]);
        }

        else if (!wcscmp (L"-median", m_ppArgv[i]))
        {
            //Median filter on or off.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_Filters.bMedian = (BOOL)_wtoi(m_ppArgv[i]);
        }
        
        else if (!wcscmp (L"-overlap", m_ppArgv[i])) 
        {
            // Overlap smoothing filter on or off.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_Filters.bOverlapSmoothing  = (BOOL)_wtoi(m_ppArgv[i]);
        }

        else if (!wcscmp (L"-noiseedge", m_ppArgv[i]))
        {
            //Noise edge removal on or off.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_Filters.bNoiseEdgeRemoval = (BOOL)_wtoi(m_ppArgv[i]);
        }

        //////////////////  ME_PROPERTIES  ////////////////////

        else if (!wcscmp (L"-deltamvrange", m_ppArgv[i])) 
        {
            // Delta MV range index.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_MEProps.dwDeltaMVRangeIndex = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_MEProps.dwDeltaMVRangeIndex  > 3)
            {
                wprintf_s (L"\nERROR: Invalid delta MV range index: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 3);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-motionsearchlevel", m_ppArgv[i])) 
        {
            // Chroma search.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_MEProps.dwMotionSearchLevel = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_MEProps.dwMotionSearchLevel < 0 || m_MEProps.dwMotionSearchLevel > 4)
            {
                wprintf_s (L"\nERROR: Invalid motion search level: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 4);
                return FALSE;
            }
        } 

        else if (!wcscmp (L"-mesearchmethod", m_ppArgv[i])) 
        {
            // Motion matching.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_MEProps.dwMotionSearchMethod = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_MEProps.dwMotionSearchMethod > 2)
            {
                wprintf_s (L"\nERROR: Invalid search method: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 2);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-mbcost", m_ppArgv[i])) 
        {
            // Macroblock cost mode.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_MEProps.dwMBModeCost =  wcstoul(m_ppArgv[i], NULL, 10);

            if (m_MEProps.dwMBModeCost > 1)
            {
                wprintf_s (L"\nERROR: Invalid m_uiMacroblockCost: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 1);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-mvcost", m_ppArgv[i])) 
        {
            // Motion vector cost mode.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_MEProps.dwMVCost =  wcstoul(m_ppArgv[i], NULL, 10);

            if (m_MEProps.dwMVCost > 1)
            {
                wprintf_s (L"\nERROR: Invalid m_uiMotion Vector Cost: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 1);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-mvrange", m_ppArgv[i])) 
        {
            // MV range.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_MEProps.dwMVRange = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_MEProps.dwMVRange  > 4)
            {
                wprintf_s (L"\nERROR: Invalid MV range: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 4);
                return FALSE;
            }
        }

        ///////////////////////  QUANT_PROPERTIES /////////////////////

        else if (!wcscmp (L"-adaptivequant", m_ppArgv[i])) 
        {
            // Adaptive quantization.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_QuantProps.dwAdaptiveQuant = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_QuantProps.dwAdaptiveQuant > 21)
            {
                wprintf_s (L"\nERROR: Invalid adaptivequant: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 21);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-dquantoption", m_ppArgv[i])) 
        {
            // Perceptual encoding.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_QuantProps.dwDQuantOption = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_QuantProps.dwDQuantOption > 3)
            {
                wprintf_s (L"\nERROR: Invalid dquantoption level: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 3);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-dquantpstrength", m_ppArgv[i])) 
        {
            // DQuant P strength.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_QuantProps.dwDQuantPStrength = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_QuantProps.dwDQuantPStrength > 2)
            {
                wprintf_s (L"\nERROR: Invalid dquantpstrength: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 2);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-dquantbstrength", m_ppArgv[i])) 
        {
            // DQuant B strength.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_QuantProps.dwDQuantBStrength = wcstoul(m_ppArgv[i], NULL, 10);

            if (m_QuantProps.dwDQuantBStrength > 4)
            {
                wprintf_s (L"\nERROR: Invalid dquantbstrength: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 4);
                return FALSE;
            }
        }

        ////////////////////  Other advanced settings  ///////////////////////

        else if (!wcscmp (L"-bdeltaqp", m_ppArgv[i])) 
        {
            // B delta QP.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_dwBframeDeltaQP =  wcstoul(m_ppArgv[i], NULL, 10);

            if (m_dwBframeDeltaQP > 30)
            {
                wprintf_s (L"\nERROR: Invalid bdeltaqp: %s.  Minimum %d, Maximum %d.\n", m_ppArgv [i], 0, 30);
                return FALSE;
            }
        } 

        else if (!wcscmp (L"-pr", m_ppArgv[i]) )
        {
            // Use audio profile.
            i++;

            if (i == m_iArgc)
            {
                wprintf (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            wcscpy_s(m_wszProfileFilePath, MAX_PATH, m_ppArgv[i]);

            size_t iFileLen = wcslen(m_wszProfileFilePath);

            if (_wcsicmp(&m_wszProfileFilePath[iFileLen-3], L"prx") != 0)
            {
                // We don't need to fail, but we should warn the user that a failure
                // is probably coming.
                wprintf (L"\nWARNING: Unexpected file name extension for profile file.\n");
                wprintf (L"\tExpected .prx or \"default\".\n\n");
            }
        }  

        else if (!wcscmp (L"-closedentrypt", m_ppArgv[i])) 
        {
            // Closed entry point.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_bClosedEntryPt = (BOOL)_wtoi(m_ppArgv[i]);
        }

        else if (!wcscmp (L"-sarx", m_ppArgv[i]))
        {
            // SAR x value.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            dwSARx = wcstoul(m_ppArgv[i], NULL, 10);
        }

		else if (!wcscmp (L"-sary", m_ppArgv[i]))
        {
            // SAR y value.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            dwSARy = wcstoul(m_ppArgv[i], NULL, 10);
        }

		else if (!wcscmp (L"-dw", m_ppArgv[i]))
        {
            // Display width.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwDisplayWidth = wcstoul(m_ppArgv[i], NULL, 10);
        }

        else if (!wcscmp (L"-dh", m_ppArgv[i]) )
        {
            // Display height.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwDisplayHeight = wcstoul(m_ppArgv[i], NULL, 10);
        }

        else if (!wcscmp (L"-ew", m_ppArgv[i]))
        {
            // Encoded width.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwEncodeWidth = wcstoul(m_ppArgv[i], NULL, 10);
        }

        else if (!wcscmp (L"-eh", m_ppArgv[i]) )
        {
            // Encoded height.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwEncodeHeight = wcstoul(m_ppArgv[i], NULL, 10);
        } 

        else if (!wcscmp (L"-lbpresent", m_ppArgv[i])) 
        {
            // Letterbox present.
            m_bLBPresent = TRUE;
        }

        else if (!wcscmp (L"-lookahead", m_ppArgv[i])) 
        {
            // Look ahead. 
            m_bLookahead =  TRUE;
        }

		// TT show encoder parms
		else if (!wcscmp (L"-showparms", m_ppArgv[i])) 
        {
            // Show encoder parms
            m_bShowParms =  TRUE;
        }

        else if (!wcscmp (L"-adaptivegop", m_ppArgv[i])) 
        {
            // Adaptive GOP.           
            m_bAdaptiveGOP =  TRUE;
        }

        else if (!wcscmp (L"-keypop", m_ppArgv[i])) 
        {
            // Key frame pulse reduction. 
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwKeyPop = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_dwKeyPop > 4)
            {
                wprintf_s(L"\nERROR: Invalid number of threads: %s. Maximum is %d.", m_ppArgv[i], 4);
                return FALSE;
            }
        } 

        else if (!wcscmp (L"-threads", m_ppArgv[i]) )
        {
            // Thread count.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwNumThreads = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_dwNumThreads > 4)
            {
                wprintf_s(L"\nERROR: Invalid number of threads: %s. Maximum is %d.", m_ppArgv[i], 4);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-affinity", m_ppArgv[i]) )
        {
            // Affinity mask.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwAffinityMask = wcstoul(m_ppArgv[i], NULL, 10);
        }

        else if (!wcscmp (L"-aspectratio", m_ppArgv[i]))
        {
            // Pixel aspect ratio.
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            // Convert the argument to a number.
            m_dwPixIndex = wcstoul(m_ppArgv[i], NULL, 10);

            // 15 means "Aspect width and height transmitted"
            // Update the w and h based on command line args.
            if (m_dwPixIndex == 15)
            {
                i++;

                if (i == m_iArgc)
                {
                    wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                    return FALSE;
                }

                m_bPARI = TRUE;

                // Convert the argument to a number.
                m_dwPixWidth = wcstoul(m_ppArgv[i], NULL, 10);

                i++;

                if (i == m_iArgc) 
                {
                    wprintf_s (L"\nERROR: You must provide argument(s) for %s.\n", m_ppArgv [i - 1]);
                    return FALSE;
                }

                // Convert the argument to a number.
                m_dwPixHeight = wcstoul(m_ppArgv[i], NULL, 10);                
            }           
        }

        else if (!wcscmp (L"-videotype", m_ppArgv[i]) )
        {
            // Frame detection logic
            i++;

            if (i == m_iArgc)
            {
                wprintf_s (L"\nERROR: You must provide an argument for %s.\n", m_ppArgv [i - 1]);
                return FALSE;
            }

            m_dwVideoType = wcstoul(m_ppArgv[i], NULL, 10);

            if(m_dwVideoType > 4)
            {
                wprintf_s(L"\nERROR: Invalid video type: %s. Maximum is %d.", m_ppArgv[i], 4);
                return FALSE;
            }
        }

        else if (!wcscmp (L"-tff", m_ppArgv[i]))
        {
            // Top field first flag.
            m_bTFF = TRUE;
        }

        else if (!wcscmp (L"-rff", m_ppArgv[i]))
        {
            // Repeat first field flag.
            m_bRFF = TRUE;
        }

        // Not looking for these on this pass, but we need to deal with them gracefully.

        // These take an argument.
        else if (!wcscmp (L"-i", m_ppArgv[i]) ||
                 !wcscmp (L"-o", m_ppArgv[i]) ||
                 !wcscmp (L"-rate", m_ppArgv[i]) ||
                 !wcscmp (L"-complexity", m_ppArgv[i]) ||
                 !wcscmp (L"-qp", m_ppArgv[i]) ||
                 !wcscmp (L"-maxkeydist", m_ppArgv[i]) ||
                 !wcscmp (L"-bframes", m_ppArgv[i]) ||
                 !wcscmp (L"-peakrate", m_ppArgv[i]) ||
                 !wcscmp (L"-profiletype", m_ppArgv[i]) ||
                 !wcscmp (L"-ratecontrol", m_ppArgv[i]) ||
                 !wcscmp (L"-vbv", m_ppArgv[i]))
        {
            i++;
        }

        // This one takes no arguments.
        else if (!wcscmp (L"-interlaced", m_ppArgv[i]))
        {
            // Just ignore it.            
        }

        else
        {
            // Unknown option.
            wprintf_s (L"\nERROR: Unknown option %s.\n", m_ppArgv[i]);
      
            //PrintHelp();
            return FALSE;
        }
    } 

#ifdef _DEBUG
    _CrtSetDebugFillThreshold(sizeOld);
#endif

	// TT Validate filter options
	wprintf_s (L"\nINFO: Validate FILTERS\n");

	if (m_ConfigProps.dwProfile == 0 && m_Filters.bInLoop == 1)
	{
		wprintf_s (L"INFO: In-Loop filter not available for simple profile - turned off.\n");
		m_Filters.bInLoop = 0;
	}

	wprintf_s (L"INFO: Validate FILTERS - done\n\n");

	// Validate other properties
	wprintf_s (L"\nINFO: Validate Other Properties\n");

	// TT Set display from sar parms, based on input AVI size
	if (dwSARx == 0 && dwSARy != 0)
	{
		wprintf_s (L"ERROR: sary is set but not sarx.\n");
		return FALSE;
	}
	if (dwSARx != 0 && dwSARy == 0)
	{
		wprintf_s (L"ERROR: sarx is set but not sary.\n");
		return FALSE;
	}
	if (dwSARx != 0 && dwSARy != 0)
	{
		m_bPARI = TRUE;
		m_dwPixIndex = 15;
		m_dwPixWidth = dwSARx;
		m_dwPixHeight = dwSARy;

		if (dwSARx > dwSARy)
		{
			m_dwDisplayHeight = m_ConfigProps.dwMaxHeight;
			m_dwDisplayWidth = int(floor((float(m_ConfigProps.dwMaxWidth) * dwSARx / dwSARy) + 0.5));
		}
		else
		{
			m_dwDisplayWidth = m_ConfigProps.dwMaxWidth;
			m_dwDisplayHeight = int(floor((float(m_ConfigProps.dwMaxHeight) * dwSARy / dwSARx) + 0.5));
		}

		if (m_dwDisplayWidth == 853 && m_dwDisplayHeight == 480)
		{
			m_dwDisplayWidth = 852;
			m_dwDisplayHeight = 480;
		}
	}

	// TT Reset m_dwBframeDeltaQP if it's not going to be used
	if (m_ConfigProps.dwProfile == 0 && m_dwBframeDeltaQP != 0)
	{
		wprintf_s (L"INFO: BframeDeltaQP not used for simple profile - Setting to 0.\n");
		m_dwBframeDeltaQP = 0;
	}
	if (m_ConfigProps.dwNumOfBFrames == 0 && m_dwBframeDeltaQP != 0)
	{
		wprintf_s (L"INFO: BframeDeltaQP not used without b-frames - Setting to 0.\n");
		m_dwBframeDeltaQP = 0;
	}

	wprintf_s (L"INFO: Validate Other Properties - done\n\n");

    return TRUE;
}

////////////////////////////////////////////////////////
//
//  CVC1EncASF::ShowStatistics
//
//  Description: Writes a stats line to the console window.
//
////////////////////////////////////////////////////////
DWORD CVC1EncASF::ShowStatistics()
{
    DWORD dwError = ERR_OK;
    DOUBLE dAvgBitRate = 0.0;
    DOUBLE dTotalBytes = 0.0;
    DOUBLE dTotalFrames = 0.0;
    DOUBLE dCodedFrames = 0.0;

    dwError = m_pEncoder->GetWriterStatistics(&dAvgBitRate, &dTotalBytes, &dTotalFrames, &dCodedFrames);

    if(ERR_OK == dwError)
    {
        wprintf_s(L"Average bit rate: %6.2f, total bytes processed: %10.2f.\n", dAvgBitRate, dTotalBytes);
        wprintf_s(L"Total frames: %6.2f, Coded frames: %6.2f.\n\n", dTotalFrames, dCodedFrames);
    }    

    return dwError;
}


