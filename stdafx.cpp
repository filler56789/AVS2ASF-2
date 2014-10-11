// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.

// stdafx.cpp : source file that includes just the standard includes
// AVI2ASF.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


// Prints the command line options to the console window.
void PrintHelp()
{
    wprintf_s (L"\n\nAVS2ASF Sample Help\n");
    wprintf_s (L"Original Sample AVI2ASF code Copyright (C) Microsoft Corporation.\n");
	wprintf_s (L"All rights reserved.\n");
	wprintf_s (L"\nAvisynth, YV12 & Raw support added by Nic.\n\n");
    wprintf_s (L"\n");

	// TT
	wprintf_s (L"The only required parameters are the input and output files, although results\n");
	wprintf_s (L"may not be what you want! Check CONFIG_PROPERTIES (using -showparms) in the\n");
	wprintf_s (L"job output and adjust any parameters accordingly.\n\n");

    wprintf_s (L"Syntax: \nAVS2ASF.exe -i <inputfile> -o <outputfile>\n");
	wprintf_s (L"\t[Option 1] <value1> [Option 2] <value2> ...\n");
    wprintf_s (L"\n\nCommand line options:\n\n");

    wprintf_s (L"-i <input AVI file name>\n");
    wprintf_s (L"\n");

    wprintf_s (L"-o <output file name>\n");
	wprintf_s (L"\tUse suffix .vc1 for VC1 Elementry Stream encoding (no audio).\n");
	wprintf_s (L"\tUse suffix .asf for ASF output.\n");
    wprintf_s (L"\n");

    ///////////////  CONFIG_PROPERTIES  ///////////////////

    wprintf_s (L"[-rate] <output rate (kilobits per second)>\n");
    wprintf_s (L"\n");

     wprintf_s (L"[-complexity] <encoder complexity>\n");
    wprintf_s (L"\t0 to 5, 0 = best performance, 5 = best quality.\n");
    wprintf_s (L"\n");
 
    wprintf_s (L"[-qp] <quantization parameter (QP)>\n");
    wprintf_s (L"\tFixed QP if -ratecontrol equals 1 (1-pass VBR)\n");
    wprintf_s (L"\tMax QP if -ratecontrol equals 0 or 2 (CBR)\n");
    wprintf_s (L"\n"); 

    //wprintf_s (L"[-framerate] <source frame rate>\n");
    //wprintf_s (L"\n");

    wprintf_s (L"[-interlaced]\n\tSpecifies interlaced source.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-maxkeydist] <maximum distance between key frames>\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-bframes] <number of B-frames per P-frame>\n");
    wprintf_s (L"\tRange is 0 to 7.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-peakrate] <peak bit rate (kilobits per second)>\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-profiletype] <type specifier>\n");
    wprintf_s (L"\t0 = Simple, 1= Main, 2 = Advanced.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-ratecontrol]\n");
    wprintf_s(L"\t0 = 1-pass CBR\n\t1 = 1-pass VBR fixed QP\n");
    wprintf_s (L"\t2 = 2-pass CBR\n\t3 = 2-pass peak constrained VBR\n\t4 = 2-pass unconstrained VBR.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-vbv] <size of vbv buffer>\n");
    wprintf_s (L"\tMaximum is 4294967295\n");
    wprintf_s (L"\n");

    ////////////////// COLOR_FORMAT ///////////////////////

    wprintf_s (L"[-colorformatflags] <colorformatflag> <colorprimaries> <transferchar> \n\t\t    <matrixcoef>\n");
    wprintf_s (L"\t- If colorformatflag is 1, then you must specify colorprimaries, \n\t  transferchar, and matrixcoef.\n");
    wprintf_s (L"\t- If colorformatflag is 0, then no need to specify colorprimaries,\n");
    wprintf_s (L"\t  transferchar, and matrixcoef because the decoder will generate them\n\t  for you.\n");
    wprintf_s (L"\n");


    /////////////////// FILTERS /////////////////////////

    wprintf_s (L"[-denoise] <0 or 1>\n");
    wprintf_s (L"\tTurn on or off denoise filter.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-inloop] <0 or 1>\n");
    wprintf_s (L"\tTurn on or off in-loop filter.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-median] <0 or 1>\n");
    wprintf_s (L"\tTurn on or off median filter.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-overlap] <0 or 1>\n");
    wprintf_s (L"\tTurn on or off overlap smoothing.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-noiseedge] <0 or 1>\n");
    wprintf_s (L"\tTurn on or off noise edge removal.\n");
    wprintf_s (L"\n");

    //////////////////  ME_PROPERTIES  ////////////////////
    
    wprintf_s (L"[-deltamvrange] <delta mv range index>\n");
    wprintf_s (L"\t0 = off, 1= horizontal, 2= vertical, 3 = h + v.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-motionsearchlevel] <level>\n"); 
    wprintf_s (L"\t0 = Luma only. (Default)\n\t1 = Luma w/ nearest-int chroma. \n\t2 = Luma w/ true chroma.\n");
    wprintf_s (L"\t3 = Macroblock-adaptive with true chroma. \n\t4 = Macroblock adaptive with nearest int chroma.\n");
    wprintf_s (L"\n");   

    wprintf_s (L"[-mesearchmethod] <phase>\n"); 
    wprintf_s (L"\t0 = off, 1 = always use hadamard, 2 = adaptive sad/hadamard.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-mbcost] <cost method>\n");
    wprintf_s (L"\t0 = SAD/Hadamard, 1 = RD cost.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-mvcost] <cost method>\n");
    wprintf_s (L"\t0 = static, 1 = dynamic.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-mvrange] <mv range>\n");
    wprintf_s (L"\t0 to 3 are increasing ranges. 4 is macroblock adaptive.\n");
    wprintf_s (L"\n");

    ///////////////////////  QUANT_PROPERTIES /////////////////////


    wprintf_s (L"[-dquantoption] <level>\n");
    wprintf_s (L"\t0 to 3, 0 = off, 1 = I frame only, 2 = I/P frame only, 3 = I/P/B frame.\n");
    wprintf_s (L"\n");

     wprintf_s (L"[-adaptivequant] <adaptive quantization>\n");
    wprintf_s (L"\t0 to 21\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-dquantpstrength] <level>\n");
    wprintf_s (L"\tRange is 0 to 2.\n");
    wprintf_s (L"\t0 = VC-1 encoder-managed setting.\n");
    wprintf_s (L"\tOnly valid when -dquantoption > 0. \n");
    wprintf_s (L"\n");

    wprintf_s (L"[-dquantbstrength] <level>\n");
    wprintf_s (L"\tRange is 0 to 4.\n");
    wprintf_s (L"\t0 = VC-1 encoder-managed setting.\n");
    wprintf_s (L"\tOnly valid when -dquantoption > 0. \n");
    wprintf_s (L"\n");

    ////////////////////  Other advanced settings  ///////////////////////

    wprintf_s (L"[-bdeltaqp] <qp increase for B-frames>\n");
    wprintf_s (L"\t0 to 30\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-closedentrypt] <closed entry point value.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-dw] <display width (pixels)>\n");
    wprintf_s (L"\n");    

    wprintf_s (L"[-dh] <display height (pixels)>\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-ew] <encode width (pixels)>\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-eh] <encode height (pixels)>\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-lbpresent]\n"); 
    wprintf_s (L"\tIndicates letterboxing is present.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-lookahead]\n");
    wprintf_s (L"\tIndicates use lookahead.\n");
    wprintf_s (L"\t-ratecontrolmode must equal 0.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-adaptiveGOP]\n");
    wprintf_s (L"\tIndicates use adaptive GOP.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-keyPop] <key pop level>\n");
    wprintf_s (L"\tIndicates use key frame pulse reduction.\n");
    wprintf_s (L"\tRange is 0 to 4. 0 = Off, 1 -4 are strength levels.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-threads] <num threads>\n");
    wprintf_s (L"\tMaximum is 4.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-affinity] <affinity mask>\n");
    wprintf_s (L"\tProcessor mapping.\n");
    wprintf_s (L"\n");    

    wprintf_s (L"[-aspectratio] <aspectratioindex> <aspectratiowidth> <aspectratioheight> \n");
    wprintf_s (L"\tIf aspectratioindex is 15, then you must specify aspectratiowidth and\n\taspectratioheight.\n");
    wprintf_s (L"\n");

	wprintf_s (L"[-sarx] <sar x value> [-sary] <sar y value>\n");
    wprintf_s (L"\tIf sar values are specified then they override -aspectratio,\n\t-dw and -dh.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-videotype] <type>\n");
    wprintf_s (L"\t0 = progressive, 1 = interlaced frames, 2 = interlaced fields\n");
    wprintf_s (L"\t3 = detect interlacing type, 4 = automatic.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-tff]\n"); 
    wprintf_s (L"\tIndicates top field first.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-rff]\n"); 
    wprintf_s (L"\tIndicates repeat first field.\n");
    wprintf_s (L"\n");

    wprintf_s (L"[-pr] <path to .prx file>\n"); 
    wprintf_s (L"\tSpecifies a .prx file to use for audio profile data.\n");
    wprintf_s (L"\tBy default, the sample looks for an appropriate audio codec.\n");
    wprintf_s (L"\n");

	wprintf_s (L"[-showparms]\n"); 
    wprintf_s (L"\tShows the parameters used by the encoder.\n");
    wprintf_s (L"\n");

	// Nic
    wprintf_s (L"\n\nExample: AVS2ASF.exe -i video.avs -o encoded.asf -ratecontrol 3\n");
    wprintf_s (L"\t-rate 1200 -peakrate 2000 -profiletype 2 -bframes 2 -vbv 300000\n");

}
