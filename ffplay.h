 /************************************************************************
* File Name : ffplay.h
* Copyright :  OS-easy Corporation, All Rights Reserved.
* Module Name : ffplay Process
*
* Create Date : 2015/12/30
* Author/Corporation : CaoLinfeng/OS-easy
*
*Abstract Description : ffplay Process
*
----------------------------------------Revision History---------------------------------
* No     Version  	 Date      		Revised By		 Item		 		        Description
* 1       V0.1    2015/12/30       CaoLinfeng 	   VDI Project        	      ffplay Process
*
************************************************************************/

/************************************************************************
* Multi-Include-Prevent Section
************************************************************************/
#ifndef FFPLAY_H  
#define FFPLAY_H  

/************************************************************************
* Include File Section
************************************************************************/ 
#include <semaphore.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avio.h>
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavutil/mem.h"
#include "libavutil/avstring.h"
#include "libavutil/samplefmt.h"
#include "define_ffplay.h"

/************************************************************************
* Prototype Declare Section
************************************************************************/
/* Called from the main */
int ffplay_init(int argc, char **argv);

//Get VideoState
VideoState* get_videostate_for_gtk();

//Set VideoState
void set_videostate_for_gtk(VideoState  *status);

//Pause
void toggle_pause(VideoState *is);

  
#endif/*FFPLAY_H*/