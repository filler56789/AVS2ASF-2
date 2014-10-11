//*****************************************************************************
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//  ASFReader.cpp
//
//  Implementation of CASFFileReader.
//
//*****************************************************************************
#include "stdafx.h"
#include "AVIReader.h"


CAVIFileReader::CAVIFileReader()
{
    // Initialize member variables.
    m_iCurrentFrameNumber = 0;
    m_aviFileIn = NULL;   
    m_videoStream = NULL;
    m_audioStream = NULL;
    m_pWavInfoHdr = NULL;       
    m_pbiHeaderIn = NULL; 
    m_dwFrames = 0;
    m_dwFirstFrame = 0;
    m_iAudioBufferSize = 0;
    m_dwAudioSamplesPos = 0;
    m_iAudioSamplesPerFrame = 0;
    m_dwLastAudio = 0; // Nic
    m_dwFirstAudio = 0;
    m_bVideoStream = FALSE;
    m_bAudioStream = FALSE;    
    m_pVideoBufferIn = NULL;    
    m_pAudioBufferIn = NULL; 
    m_pwszFileName = NULL;
	m_bSwapUV = false; // Nic

    ZeroMemory(&m_aviFileInInfo, sizeof(AVIFILEINFO));
    ZeroMemory(&m_streamInfoVideo, sizeof(AVISTREAMINFO));
    ZeroMemory(&m_streamInfoAudio, sizeof(AVISTREAMINFO));
    ZeroMemory(&m_videoStream, sizeof(PAVISTREAM));
    ZeroMemory(&m_audioStream, sizeof(PAVISTREAM));    
}

////////////////////////////////////////////////////////////////////
//
//  CAVIFileReader::Init
//
//  Description: Initializes the CAVIFileReader
//               class. Opens the AVI file, caches file and stream
//               info, and allocates the audio input buffer.
//
/////////////////////////////////////////////////////////////////////
HRESULT CAVIFileReader::Init(WCHAR *pwszFileName, bool bEncodeRaw)
{
    HRESULT hr = S_OK;
    int iStreams = 0; // Stream count.
	PAVISTREAM aviStream;  // Pointer to an AVI stream.
    AVISTREAMINFO streamInfoIn;  // Stream info structure.

    if(NULL == pwszFileName)
    {
        return E_POINTER;
    }

    // Cache the file path.
    m_pwszFileName = _wcsdup(pwszFileName);
 
	// Initialise framerate values
	m_dwFRNum = 0;
	m_dwFRDen = 0;

    // Initialize the AVIFile library.
    AVIFileInit(); 

    // Attempt to open the AVI file.
    hr = AVIFileOpen (&m_aviFileIn, m_pwszFileName, 0, 0);

    if(SUCCEEDED(hr))
    {        
        // Retrieve the AVIFILEINFO structure for the current file.
        hr = AVIFileInfo(m_aviFileIn, &m_aviFileInInfo, sizeof(m_aviFileInInfo));        
    }

    if(SUCCEEDED(hr))
    {
        // Cache the stream count.
		iStreams = m_aviFileInInfo.dwStreams; 

        // Loop through the streams (video)
        for (int i = 0; i <iStreams; i++) 
        {
            hr = AVIFileGetStream (m_aviFileIn, &aviStream, 0, i);
  
            if(SUCCEEDED(hr))
            {
                // Retrieve the stream info structure for the current stream.
                hr = AVIStreamInfo (aviStream, &streamInfoIn, sizeof(streamInfoIn));
            }

            if(SUCCEEDED(hr))
            {           
                if (streamtypeVIDEO == streamInfoIn.fccType)
                {
                    // Video stream

                    // Only support I420 and IYUV.   
					// Nic added YV12
					if (streamInfoIn.fccHandler == FOURCC_YV12 ||
						streamInfoIn.fccHandler == FOURCC_yv12)
					{
						m_bSwapUV = true;
						streamInfoIn.fccHandler = FOURCC_I420;
					}
                    if(streamInfoIn.fccHandler != FOURCC_I420 &&
                       streamInfoIn.fccHandler != FOURCC_IYUV &&
                       streamInfoIn.fccHandler != FOURCC_iyuv &&
                       streamInfoIn.fccHandler != FOURCC_i420)
                    {
						// Nic
						wprintf_s(L"\nERROR: Bad AVS/AVI format - Has to be YV12!\n");
                        hr = AVIERR_BADFORMAT;

                        // Exit the loop.
                        break;
                    }

                    m_bVideoStream = TRUE;
                    m_streamInfoVideo = streamInfoIn;
                    m_videoStream = aviStream;
                    m_dwFrames = streamInfoIn.dwLength;
                    m_dwFirstFrame = AVIStreamStart(m_videoStream);
                    LONG videoFormatSize = 0;

					// TT get framerate
					m_dwFRNum = streamInfoIn.dwRate;
					m_dwFRDen = streamInfoIn.dwScale;

                    // Retrieve the size of the format data.
                    hr = AVIStreamReadFormat (m_videoStream, 0, NULL, &videoFormatSize);

                    if(SUCCEEDED(hr))
                    {
                        // Allocate memory for the format data.
                        m_pbiHeaderIn = (BITMAPINFOHEADER *) new char[videoFormatSize];
                        if(NULL == m_pbiHeaderIn)
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }

                        ZeroMemory(m_pbiHeaderIn, videoFormatSize);

                        // Retrieve the video format data.
                        hr = AVIStreamReadFormat(m_videoStream, 0, m_pbiHeaderIn, &videoFormatSize); 

                        if(FAILED(hr))
                        {
                            // Unable to retrieve the format data.
                            // Exit the loop.
                            break;
                        }
						else
						{
							// Nic
							m_lYSample = m_pbiHeaderIn->biWidth*m_pbiHeaderIn->biHeight;
							m_lUVSample = m_lYSample/4;
						}
                    } // if(SUCCEEDED(hr))
				} // if (streamtypeVIDEO == streamInfoIn.fccType)
			} // if(SUCCEEDED(hr))
		} // for (int i = 0; i <iStreams; i++)
	} // if(SUCCEEDED(hr))


	// TT Now get audio, unless encoding raw
    if(SUCCEEDED(hr) && bEncodeRaw == FALSE)
    {
        // Loop through the streams (audio)
        for (int i = 0; i <iStreams; i++)
        {
            hr = AVIFileGetStream (m_aviFileIn, &aviStream, 0, i);
  
            if(SUCCEEDED(hr))
            {
                // Retrieve the stream info structure for the current stream.
                hr = AVIStreamInfo (aviStream, &streamInfoIn, sizeof(streamInfoIn));
            }

            if(SUCCEEDED(hr))
            {
                if (streamtypeVIDEO != streamInfoIn.fccType)
                {
                    // Audio stream

                    m_bAudioStream = TRUE;
                    m_streamInfoAudio = streamInfoIn;
                    m_audioStream = aviStream;
                    m_dwFirstAudio = AVIStreamStart(m_audioStream);
					m_dwLastAudio = AVIStreamEnd(m_audioStream);		// Nic
                    LONG audioFormatSize = 0;

                    // Retrieve the size of the format data.
                    hr = AVIStreamReadFormat (m_audioStream, 0, NULL, &audioFormatSize);
                    
                    if(SUCCEEDED(hr))
                    {
                        // Allocate memory for the format data.
                        m_pWavInfoHdr = (WAVEFORMATEX *) new char [audioFormatSize];
                        if(NULL == m_pWavInfoHdr)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    if(SUCCEEDED(hr))
                    {
                        ZeroMemory(m_pWavInfoHdr, audioFormatSize);                            

                        // Retrieve the audio format data.
                        hr = AVIStreamReadFormat (m_audioStream, 0, m_pWavInfoHdr, &audioFormatSize);
                    }

                    if(SUCCEEDED(hr))
                    {
						// TT use video framrate if set - hopefully it is
						double dFR = 0;
						if (m_dwFRNum == 0 || m_dwFRDen == 0)
						{
							m_dwFRNum = streamInfoIn.dwRate;
							m_dwFRDen = streamInfoIn.dwScale;
						}

						dFR = m_dwFRNum / m_dwFRDen;

                        // Calculate the audio buffer size.
                        m_iAudioSamplesPerFrame = (int) (m_pWavInfoHdr->nSamplesPerSec / dFR);
                        m_iAudioBufferSize = (int) ((m_iAudioSamplesPerFrame) * (m_pWavInfoHdr->wBitsPerSample * m_pWavInfoHdr->nChannels / 8));
                        
                        // Allocate the audio input buffer.
                        m_pAudioBufferIn = new BYTE [m_iAudioBufferSize];
                        if(NULL ==  m_pAudioBufferIn)
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }

                        ZeroMemory(m_pAudioBufferIn, m_iAudioBufferSize);
                    }
                    else
                    {
                        // Unable to read the format data.
                        // Exit the loop.
                        break;
                    }
				} // if (streamtypeVIDEO != streamInfoIn.fccType)
			} // if(SUCCEEDED(hr))
		} // for (int i = 0; i <iStreams; i++)
	} // if(SUCCEEDED(hr) && bEncodeRaw == FALSE)


    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CAVIFileReader::Init\nhr = %x \n", hr);
    }

    return hr;
}

////////////////////////////////////////////////////////
//
// CAVIFileReader::GetFormat_Audio
//
// Description: Returns the audio format structure.
//
////////////////////////////////////////////////////////
HRESULT CAVIFileReader::GetFormat_Audio(WAVEFORMATEX** ppWFX) 
{
    HRESULT hr = S_OK;

    if(NULL != ppWFX)
    {   
        *ppWFX = m_pWavInfoHdr;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

////////////////////////////////////////////////////////
//
// CAVIFileReader::GetFormat_Video
//
// Description: Returns the video format structure.
//
////////////////////////////////////////////////////////
HRESULT CAVIFileReader::GetFormat_Video(BITMAPINFOHEADER** ppBIH) 
{
    HRESULT hr = S_OK;

    if(NULL != ppBIH)
    {
        *ppBIH = m_pbiHeaderIn;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

//////////////////////////////////////////////////////////////
//
//  CAVIFileReader::Destroy
//
//  Description: Closes AVI files and streams and deallocates
//               memory buffers, if they exist.
//
///////////////////////////////////////////////////////////////
HRESULT CAVIFileReader::Destroy()
{
    HRESULT hr = S_OK;

    // Clean up video.
    if (m_bVideoStream) 
    {
        AVIStreamClose(m_videoStream);        
    }

    SAFE_ARRAY_DELETE(m_pVideoBufferIn);
    SAFE_ARRAY_DELETE(m_pbiHeaderIn);


    // Clean up audio.
    if (m_bAudioStream)
    {
        AVIStreamClose (m_audioStream);            
    }

    SAFE_ARRAY_DELETE(m_pAudioBufferIn);
    SAFE_ARRAY_DELETE(m_pWavInfoHdr);

    // Close the AVI file.
    if (NULL != m_aviFileIn) 
    {
        AVIFileClose(m_aviFileIn);
        m_aviFileIn = NULL;
    }

    AVIFileExit();

    return hr;
}

////////////////////////////////////////////////////////
//
// CAVIFileReader::GetFrameCount
//
// Description: Returns the frame count for the
//              video stream.
//
////////////////////////////////////////////////////////
HRESULT CAVIFileReader::GetFrameCount(INT32 *piFrameCount)
{
    HRESULT hr = S_OK;
    
    if(NULL != piFrameCount)
    {
        *piFrameCount = m_dwFrames;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

////////////////////////////////////////////////////////
//
// CAVIFileReader::GetFramerate
//
// Description: Returns the framerate for the
//              video stream.
//
////////////////////////////////////////////////////////
HRESULT CAVIFileReader::GetFramerate(double *pdFramerate)
{
    HRESULT hr = S_OK;
    
    if(NULL != pdFramerate)
    {
        *pdFramerate = double(m_dwFRNum) / double(m_dwFRDen);
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

////////////////////////////////////////////////////////
//
// CAVIFileReader::GetFirstFrame
//
// Description: Returns the number of the first frame.
//
////////////////////////////////////////////////////////
HRESULT CAVIFileReader::GetFirstFrame(INT32 *piFirstFrame)
{
    HRESULT hr = S_OK;
    
    if(NULL != piFirstFrame)
    {
        *piFirstFrame = m_dwFirstFrame;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

////////////////////////////////////////////////////////////////////
//
//  CAVIFileReader::ReadFrame_Video
//
//  Description: Returns a pointer to a video frame and its size.
//               Returns NULL values if no video stream exists.
//
/////////////////////////////////////////////////////////////////////
HRESULT CAVIFileReader::ReadFrame_Video(BYTE **pbVideo, DWORD *pdwAVIVideoLen, INT64 *pqwTimestamp)
{
    HRESULT hr = S_OK;
    LONG lBytesRead = 0;
    LONG cbSample = 0;
    LONG cSamples = 0;

    if(NULL == pbVideo ||
       NULL == pdwAVIVideoLen ||
       NULL == pqwTimestamp)
    {
        return E_POINTER;
    }

    if (m_bVideoStream) 
    {
        // Get the buffer size.
        hr = AVIStreamRead(m_videoStream, m_iCurrentFrameNumber, AVISTREAMREAD_CONVENIENT, NULL, 0, &cbSample, &cSamples);

        if(SUCCEEDED(hr))
        {
            // Destroy any existing video buffer.
            SAFE_ARRAY_DELETE(m_pVideoBufferIn);
            // Create a new video buffer for the current frame.
            m_pVideoBufferIn = new BYTE [cbSample];
            if(NULL == m_pVideoBufferIn)
            {
                hr = E_OUTOFMEMORY;
            }

            if(SUCCEEDED(hr))
            {
                ZeroMemory(m_pVideoBufferIn, cbSample);
                
                // Read the video frame.
                hr = AVIStreamRead(m_videoStream, m_iCurrentFrameNumber, AVISTREAMREAD_CONVENIENT, 
                                  m_pVideoBufferIn, cbSample, &lBytesRead, NULL);
				
				// Nic
				if ( m_bSwapUV )
				{
					BYTE *pTempVideoBuffer = new BYTE [cbSample];
					ZeroMemory(pTempVideoBuffer, cbSample);

					memcpy(pTempVideoBuffer, m_pVideoBufferIn, m_lYSample);
					memcpy(pTempVideoBuffer + m_lYSample + m_lUVSample, m_pVideoBufferIn + m_lYSample, m_lUVSample);
					memcpy(pTempVideoBuffer + m_lYSample, m_pVideoBufferIn + m_lYSample + m_lUVSample, m_lUVSample);
					delete [] m_pVideoBufferIn;
					m_pVideoBufferIn = pTempVideoBuffer;
				}
            }
        }

        if(SUCCEEDED(hr))
        {           
            // Return the video buffer and length.
            *pbVideo = m_pVideoBufferIn;
            *pdwAVIVideoLen = lBytesRead;

            // Return the timestamp.
            *pqwTimestamp = GetTimestamp_Video(m_iCurrentFrameNumber);

            // Increment the current frame number.
            m_iCurrentFrameNumber++;
        }
    }
    else 
    {
        // Return NULL values to signal no video stream present.
        *pbVideo = NULL;
        pdwAVIVideoLen = NULL;
    }

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CAVIFileReader::ReadFrame_Video\nhr = %x \n", hr);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////
//
//  CAVIFileReader::ReadFrame_Audio
//
//  Description: Returns a pointer to an audio buffer and its size.
//               Returns NULL values if no audio stream exists.
//
/////////////////////////////////////////////////////////////////////
HRESULT CAVIFileReader::ReadFrame_Audio (BYTE **pbAudio, DWORD *pdwAVIAudioLen, INT64 *pqwTimestamp)
{
    HRESULT hr = S_OK; 
    LONG lBytesRead = 0;
    INT64 qwTimestamp = 0;

    if(NULL == pbAudio ||
       NULL == pdwAVIAudioLen ||
       NULL == pqwTimestamp)
    {
        return E_POINTER;
    }

    if (m_bAudioStream)
    {
        hr = AVIStreamRead(m_audioStream, m_dwAudioSamplesPos, m_iAudioSamplesPerFrame, 
                           m_pAudioBufferIn, m_iAudioBufferSize, &lBytesRead, NULL);

		// Nic
		if(SUCCEEDED(hr) && m_dwAudioSamplesPos <= m_dwLastAudio)
        {  
            // Return the audio buffer and size.
            *pbAudio = m_pAudioBufferIn;
            *pdwAVIAudioLen = lBytesRead;

            // Increment the audio position counter.
            m_dwAudioSamplesPos += m_iAudioSamplesPerFrame;

            *pqwTimestamp = GetTimestamp_Audio();
        }
		// Nic
        else if(AVIERR_FILEREAD == hr ||
                AVIERR_NODATA == hr || m_dwAudioSamplesPos > m_dwLastAudio )
        {
            // Return NULL values to signal no more frames.
            *pbAudio = NULL;
            *pdwAVIAudioLen = 0;
            *pqwTimestamp = 0;

            // Return a success code.
            hr = S_FALSE;
        }
    }
    else 
    {
        // Return NULL values to signal no audio stream present.
        *pbAudio = NULL;
        *pdwAVIAudioLen = 0;
        *pqwTimestamp = 0;

        // Return a success code.
        hr = S_FALSE;
    }

    if(FAILED(hr))
    {
        // Print the error message.
        wprintf_s(L"Failure in CAVIFileReader::ReadFrame_Audio\nhr = %x \n", hr);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////
//
//  CAVIFileReader::GetTimestamp_Video
//
//  Description: Returns a timestamp value for the supplied number
//               of elapsed fields.
//
/////////////////////////////////////////////////////////////////////
INT64 CAVIFileReader::GetTimestamp_Video(INT64 i64TotalFrames)
{
    INT64 i64ret = 0;
 
    if (i64TotalFrames > 0) 
    {
        i64ret = (INT64)(((double)m_dwFRDen * (double)i64TotalFrames * 10000000.0) /
                 ((double)m_dwFRNum) + 0.5);
    }

    return i64ret;
}

////////////////////////////////////////////////////////////////////
//
//  CAVIFileReader::GetTimestamp_Audio
//
//  Description: Returns a timestamp value for the current audio
//               position.
//
/////////////////////////////////////////////////////////////////////
INT64 CAVIFileReader::GetTimestamp_Audio()
{
    INT64 i64ret =   0;
    DWORD dwCurrentPosition = 0; 

    assert(NULL != m_pWavInfoHdr);
    
    if (m_dwAudioSamplesPos != 0)
    {
        // Because the current position was incremented when reading the audio sample, 
        // decrement the current position by m_iAudioSamplesPerFrame to get the actual position.
        dwCurrentPosition = m_dwAudioSamplesPos - m_iAudioSamplesPerFrame;

        // Time Stamp = Current Sample Position / Samples Per Sec * Scaling Factor 
        i64ret = (INT64)(dwCurrentPosition / ((*m_pWavInfoHdr).nSamplesPerSec * 1.0) * 10000000.0); 
    }

    return i64ret;
}

////////////////////////////////////////////////////////////////////
//
//  CAVIFileReader::SeekFirst
//
//  Description: Changes the frame and audio number to the first positions.
//
/////////////////////////////////////////////////////////////////////
void CAVIFileReader::SeekFirst()
{
    m_iCurrentFrameNumber = m_dwFirstFrame;
    m_dwAudioSamplesPos = m_dwFirstAudio;

    return;
}




