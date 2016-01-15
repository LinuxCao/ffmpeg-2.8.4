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

 /* Start playing video*/  
static void *playeropen_thread(char *file);

/* Video progress bar callback function */  
static void video_seek_value_changed(GtkRange *range, gpointer data);

/* Update playback time callback function */
static gboolean update_time_callback();

/* voice progress bar callback function */  
static void voice_seek_value_changed(GtkRange *range, gpointer data);

/* Play or pause callback function */   
static void toggle_play_pause_button_callback (GtkWidget *widget, gpointer data);

/* voice or slience callback function */  
static void toggle_voice_slience_button_callback (GtkWidget *widget, gpointer data);

/* close callback function */  
void toggle_close_button_callback(GtkWidget *widget, gpointer data);

/* keyborad callback function */  
static gboolean on_main_window_key_press_event (GtkWidget *widget,GdkEventKey *event,gpointer user_data);

/* Destroy window*/  
static gint delete_event( GtkWidget *widget,GdkEvent *event,gpointer data );

/* Load video file*/  
static gboolean load_file(gchar *uri);

/* Create GUI interface*/  
static GtkWidget *build_gui();

#endif 
