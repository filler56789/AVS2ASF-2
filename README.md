========================================================================
    CONSOLE APPLICATION : AVI2ASF Project Overview    
    
    Copyright (C) Microsoft Corporation. All rights reserved.
========================================================================

December 2007

Description
-----------------------------------------------------------------------------------------------

The AVI2ASF sample demonstrates how to combine audio and VC-1 encoded video in an ASF container.


Prerequisites
-------------------------------------------------------------------
- Microsoft VC-1 Encoder SDK (Pro and Live Web)

- Microsoft Windows Media Format SDK 9.5 or later

http://msdn2.microsoft.com/en-us/windowsmedia/bb190309.aspx

- Microsoft Visual Studio 2005 SP1.


Note: Internet addresses are subject to change.


Required Files
-------------------------------------------------------------------
AVIReader.h - Declarations for the CAVIFileReader class.
AVIReader.cpp - Implementation of CAVIFileReader. Uses Video for Windows to read AVI files.
ASFWriter.h - Declarations for the CASFFileWriter class.
ASFWriter.cpp - Implementation of CASFFileWriter. Uses Windows Media Format SDK to write ASF files.
VC1EncASF.h - Declarations for the sample main class, CVC1EncASF. 
VC1EncASF.cpp - Implementation of CVC1EncASF. Manages reading, encoding, and writing operations.
AVI2ASF.cpp - Main entry point for the console application.
stdafx.h - Project wide includes. 
stdafx.cpp - Contains the PrintHelp function for command line help.



Building the Sample
-------------------------------------------------------------------
1. Install the prerequisites.

2. Configure Visual Studio. Make sure your VC++ Include and Lib directories point to the include and lib folders for the Format SDK and VC-1 Pro SDK. In most versions of Visual Studio, you set these directories by clicking Tools, then Options, expanding Projects and Solutions, and then clicking VC++ Directories.

3. Open AVI2ASF.sln.

4. Build the solution.



Running the Sample
-------------------------------------------------------------------
The sample opens an AVI file, encodes the video using the VC-1 Pro Encoder SDK, and combines the audio stream with the new VC-1 video stream using the Windows Media Format SDK. The output file is a Windows Media video file.

To understand the command line options, see the code in VC1EncASF.cpp or the PrintHelp() function in stdafx.cpp. Note that all VC-1 Pro Encoder SDK functionality might not be demonstrated by the sample code.
--------------------------------------------------------------------






