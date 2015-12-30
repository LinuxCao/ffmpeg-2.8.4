 /************************************************************************
* File Name : main.h
* Copyright :  OS-easy Corporation, All Rights Reserved.
* Module Name : Main Process 
*
* Create Date : 2015/12/30
* Author/Corporation : CaoLinfeng/OS-easy
*
*Abstract Description : Simple media player based on ffplay and GTK 
*
----------------------------------------Revision History---------------------------------
* No     Version  	 Date      		Revised By		 Item		 		        Description
* 1       V0.1    2015/12/30       CaoLinfeng 	   VDI Project           Main Process For GTK and ffplay
*
************************************************************************/

/************************************************************************
* Multi-Include-Prevent Section
************************************************************************/
#ifndef MAIN_H  
#define MAIN_H  
 
/************************************************************************
* Include File Section
************************************************************************/ 
#include <gtk/gtk.h>  
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
void gui_status_update(PlayerState state);  
void gui_update_time(const gchar *time, const gint64 position, const gint64 length);  
gboolean load_file(const gchar *uri);  
void seek_to(gdouble percentage);  
gboolean play_file();
  
#endif 
