
 /************************************************************************
* File Name : main.c
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
* Include File Section
************************************************************************/
#include <gdk/gdkx.h>  
#include <gtk/gtk.h>  
#include <string.h>  
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "main.h"  
#include "ffplay.h" 
#include "define_ffplay.h"
#include <gdk/gdkkeysyms.h>

  
/************************************************************************
* Static Variable Define Section
************************************************************************/
static GtkWidget *main_window;  
static GtkWidget *total_time_label;  //总播放的时间标签
static GtkWidget *play_time_label;  //正在播放的时间标签
static GtkWidget *blank_space_label; //空白标签:目的是分隔音量条和窗口边界为10个像素点  
static GtkWidget *seek_scale;  
static GtkWidget *video_output; 
static GtkWidget *voice_scale;  
static GtkWidget *main_vbox;  
static GtkWidget *play_controls_hbox;   
static GtkWidget *status_controls_hbox;
//static GtkWidget *video_title_label;  //视频标题标签

static GtkWidget *close_button;   //停止按钮
static GtkWidget *play_button;   //播放暂停按钮
//static GtkWidget *rewind_button;  //快退按钮
//static GtkWidget *forward_button; //快进按钮
//static GtkWidget *fullscreen_button; //全屏按钮
static GtkWidget *voice_slience_button; //静音按钮
static GtkObject *video_schedule_adj;//video  schedule adjustment
static GtkObject *voice_schedule_adj;//voice  schedule adjustment
static GdkScreen* gtk_screen;  //gtk screen 
static gint gtk_screen_width=0;  //gtk screen resolution:width
static gint gtk_screen_height=0; //gtk screen resolution:heigth
static GtkAccelGroup *play_button_accelerate;



static char *current_filename = NULL;  
static char total_time_label_string[32]={0};
static char play_time_label_string[32]={0};
static guint timeout_source = 0;   
pthread_t playeropen_msg_process_thread_tid; 				//视频消息处理线程
gboolean seek_flag = FALSE;  
gboolean play_button_status = FALSE;  //播放暂停标志位：1-播放 0-暂停 
gboolean voice_slience_button_status = FALSE;  //声音和静音标志位：1-声音 0-静音 

// initialize play_controls_hbox、 status_controls_hbox and video_output'size 
gint play_controls_hbox_width=0, play_controls_hbox_height=0;
gint status_controls_hbox_width=0, status_controls_hbox_height=0;
gint video_output_width=0, video_output_height=0;

/************************************************************************
* Function Define Section
************************************************************************/

//Get video_output_height
int get_video_output_height()
{
	return video_output_height;
}

//Set video_output_height
void set_video_output_height(int value)
{
	video_output_height = value;
}

//Get play_button_status
gboolean get_play_button_status()
{
	return play_button_status;
}

//Set play_button_status
void set_play_button_status(gboolean value)
{
	play_button_status = value;
}

//Get voice_slience_button_status
gboolean get_voice_slience_button_status()
{
	return voice_slience_button_status;
}

//Set voice_slience_button_status
void set_voice_slience_button_status(gboolean value)
{
	voice_slience_button_status = value;
}



 /* Start playing video*/  
void *playeropen_thread(char *file)
{
    //g_print("playeropen_thread start\n"); 
	char *pfile[2];
	pfile[0]="ffplay";
	pfile[1]= file;
	ffplay_init(2,pfile);
	
	return NULL;
	
}

/* Video progress bar callback function */  
void video_seek_value_changed(GtkRange *range, gpointer data)  
{  
	g_print("video_seek_value_changed\n"); 
	
	VideoState* cur_stream;
	cur_stream=get_videostate_for_gtk();
	//printf("cur_stream=0x%1x\n",cur_stream);
	//printf("cur_stream->ic=0x%1x\n",cur_stream->ic);

	if((cur_stream != NULL) && (cur_stream->ic != NULL) && (seek_flag==TRUE))
	{
		int64_t ts;
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		double current_x=0.0,frac=0.0;
		
		current_x= gtk_adjustment_get_value(GTK_ADJUSTMENT (video_schedule_adj));
		
		//printf("cur_stream=0x%1x\n",cur_stream);
		//printf("cur_stream->ic=0x%1x\n",cur_stream->ic);
		tns  = cur_stream->ic->duration / 1000000LL;
		thh  = tns / 3600;
		tmm  = (tns % 3600) / 60;
		tss  = (tns % 60);

		frac = current_x / 100.0;
		ns   = (int)(frac * tns);
		hh   = ns / 3600;
		mm   = (ns % 3600) / 60;
		ss   = (ns % 60);
		printf("current_x=%f,frac=%f,ns=%d,tns=%d\n",current_x,frac,ns,tns);
		av_log(NULL, AV_LOG_INFO,
		"Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)\n", frac*100,
		hh, mm, ss, thh, tmm, tss);
		ts = frac * cur_stream->ic->duration;
		if (cur_stream->ic->start_time != AV_NOPTS_VALUE)
		ts += cur_stream->ic->start_time;

		//刷新总的播放时间
		//获取视频总的播放时间,并格式化字符串total_time_label_string
		sprintf(total_time_label_string, "%2d:%02d:%02d", thh, tmm, tss);
		printf("total_time_label_string=%s\n",total_time_label_string);
		gtk_label_set_text(GTK_LABEL(total_time_label), total_time_label_string);  

		//刷新正在播放时间
		//获取视频总的播放时间,并格式化字符串play_time_label_string
		sprintf(play_time_label_string, "%2d:%02d:%02d", hh, mm, ss);
		printf("play_time_label_string=%s\n",play_time_label_string);
		gtk_label_set_text(GTK_LABEL(play_time_label), play_time_label_string); 

		stream_seek(cur_stream, ts, 0, 0);		
	}	
}  

/*Update playback time callback function*/
gboolean update_time_callback()  
{  
	seek_flag=FALSE;
	VideoState* cur_stream;
	cur_stream=get_videostate_for_gtk();
	//printf("cur_stream=0x%1x\n",cur_stream);
	//printf("cur_stream->ic=0x%1x\n",cur_stream->ic);
	
	if((cur_stream != NULL) && (cur_stream->ic != NULL))
	{
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		double frac=0;
		double current_x=0.0;
	
		//printf("cur_stream=0x%1x\n",cur_stream);
		//printf("cur_stream->ic=0x%1x\n",cur_stream->ic);
		
		//获取总的播放时间
		tns  = cur_stream->ic->duration / 1000000LL;
		thh  = tns / 3600;
		tmm  = (tns % 3600) / 60;
		tss  = (tns % 60);
		
		//获取视频当前播放时间
		if (isnan(get_master_clock(cur_stream)))
		{
			g_print("get_master_clock(cur_stream) == nan\n");
			return TRUE;
		}
		ns = (int)get_master_clock(cur_stream);
		hh   = ns / 3600;
		mm   = (ns % 3600) / 60;
		ss   = (ns % 60);
		
		//获进度条调整对象adjustment
		frac= ns / (tns*1.0);
		current_x=(frac * 100);
		//保留2为小数点
		current_x = ((int)(current_x*100+0.5))/100.0;
		//g_print("get_master_clock(cur_stream)= %2f,ns=%2d,tns=%2d,frac=%2f current_x=%2f(%2d:%02d:%02d) of total duration (%2d:%02d:%02d)\n",get_master_clock(cur_stream),ns,tns,frac,current_x,hh, mm, ss, thh, tmm, tss);
		
		if(ns==tns) //总的播放时间和正在播放时间一致，都为总的播放时间
		{
			g_print("End of video\n");  
			//刷新总的播放时间
			//获取视频总的播放时间,并格式化字符串total_time_label_string
			sprintf(total_time_label_string, "%2d:%02d:%02d", thh, tmm, tss);
			//g_print("total_time_label_string=%s\n",total_time_label_string);
			gtk_label_set_text(GTK_LABEL(total_time_label), total_time_label_string);  

			//刷新正在播放时间
			//获取视频总的播放时间,并格式化字符串play_time_label_string
			sprintf(play_time_label_string, "%2d:%02d:%02d", thh, tmm, tss);
			//g_print("play_time_label_string=%s\n",play_time_label_string);
			gtk_label_set_text(GTK_LABEL(play_time_label), play_time_label_string); 
			
			//同步进度条调整对象
			gtk_adjustment_set_value (GTK_ADJUSTMENT (video_schedule_adj),current_x);

		}
		else if(ns < tns )
		{
			//刷新总的播放时间
			//获取视频总的播放时间,并格式化字符串total_time_label_string
			sprintf(total_time_label_string, "%2d:%02d:%02d", thh, tmm, tss);
			//g_print("total_time_label_string=%s\n",total_time_label_string);
			gtk_label_set_text(GTK_LABEL(total_time_label), total_time_label_string);  

			//刷新正在播放时间
			//获取视频总的播放时间,并格式化字符串play_time_label_string
			sprintf(play_time_label_string, "%2d:%02d:%02d", hh, mm, ss);
			//g_print("play_time_label_string=%s\n",play_time_label_string);
			gtk_label_set_text(GTK_LABEL(play_time_label), play_time_label_string); 
			
			//同步进度条调整对象
			gtk_adjustment_set_value (GTK_ADJUSTMENT (video_schedule_adj),current_x);
		}
		//refresh screen
		//toggle_fullscreen_button_callback(GTK_WIDGET (video_output),NULL);
	}
	else
	{
		g_print("Failed to update_time_callback\n");
	}
	seek_flag=TRUE;
	return TRUE;
}  
  
/* voice progress bar callback function */  
void voice_seek_value_changed(GtkRange *range, gpointer data)  
{  
	g_print("voice_seek_value_changed\n"); 
	

	
	double volume_value = gtk_adjustment_get_value(GTK_ADJUSTMENT (voice_schedule_adj));
	if(volume_value >= 0 && volume_value <= 128)
	{
		g_print("set_default_volume_value:%2f\n",volume_value);  
		set_default_volume_value(volume_value);
	}
	if(volume_value > 0) //silence icon -> voice icon
	{
		//使用指定图标创建按钮图像
		GtkWidget* img_voice= gtk_image_new_from_file("/usr/share/ffplay/voice.png");
		//动态设置按钮的图像
		gtk_button_set_image(GTK_BUTTON(voice_slience_button),img_voice);
		//此刻为声音状态，故设置为静音的标志位，等待下次点击就是静音处理
		set_voice_slience_button_status(FALSE);
		gtk_widget_show(voice_slience_button);
	}
}  

/* Play or pause callback function */   
void toggle_play_pause_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_play_pause_button_callback\n"); 
	if(current_filename)
	{
		if (get_play_button_status()==TRUE)//play
		{
			
			g_print("GTK_STOCK_MEDIA_PLAY\n");   
			//使用指定图标创建按钮图像
			GtkWidget* img_play= gtk_image_new_from_file("/usr/share/ffplay/play.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(play_button),img_play);			
		
			
			//ffplay pause
			SDL_Event sdlevent;
			sdlevent.type = SDL_KEYDOWN;
			sdlevent.key.keysym.sym = SDLK_SPACE;
			SDL_PushEvent(&sdlevent);
			
			//此刻为播放状态，故设置为暂停的标志位，等待下次点击就是暂停处理
			set_play_button_status(FALSE);
			
			
		} 
		else //pause
		{
			g_print("GTK_STOCK_MEDIA_PAUSE\n");   
			//使用指定图标创建按钮图像
			GtkWidget* img_pause= gtk_image_new_from_file("/usr/share/ffplay/pause.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(play_button),img_pause);
	

			//ffplay play
			SDL_Event sdlevent;
			sdlevent.type = SDL_KEYDOWN;
			sdlevent.key.keysym.sym = SDLK_SPACE;
			SDL_PushEvent(&sdlevent);
			
			//此刻为暂停状态，故设置为播放的标志位，等待下次点击就是播放处理
			set_play_button_status(TRUE);
			

		}
	}
	else
	{
		g_print("please choose open video file.\n"); 
	}
}	

/* voice or slience callback function */  
void toggle_voice_slience_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_voice_slience_button_callback\n"); 
	if(current_filename)
	{
		if (get_voice_slience_button_status())//voice
		{
			g_print("voice\n");   
			//使用内置的图标创建图像
			//GtkWidget* img_play = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
			//使用指定图标创建按钮图像
			GtkWidget* img_voice= gtk_image_new_from_file("/usr/share/ffplay/voice.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(widget),img_voice);
			gtk_widget_show(widget);
			
			//recover SDL Audio
			g_print("recover SDL Audio\n");
			double volume_value = gtk_adjustment_get_value(GTK_ADJUSTMENT (voice_schedule_adj));
			if(volume_value >= 0 && volume_value <= 128)
			{
				g_print("set_default_volume_value:%2f\n",volume_value);  
				set_default_volume_value(volume_value);
			}
			
			//此刻为声音状态，故设置为静音的标志位，等待下次点击就是静音处理
			set_voice_slience_button_status(FALSE);
			

		} 
		else //slience
		{
			g_print("slience\n");   
			//使用内置的图标创建图像
			//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,GTK_ICON_SIZE_BUTTON);
			//使用指定图标创建按钮图像
			GtkWidget* img_slience= gtk_image_new_from_file("/usr/share/ffplay/slience.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(widget),img_slience);
			gtk_widget_show(widget);
			
			//Slience SDL Audio
			g_print("Slience SDL Audio :set_default_volume_value(0)\n");
			gtk_adjustment_set_value (GTK_ADJUSTMENT (voice_schedule_adj),0);
			set_default_volume_value(0);
			
			//此刻为静音状态，故设置为声音的标志位，等待下次点击就是声音处理
			set_voice_slience_button_status(TRUE);
			


		}
	}
	else
	{
		g_print("please choose open video file.\n"); 
	}
}

/* close callback function */  
void toggle_close_button_callback(GtkWidget *widget, gpointer data)
{
	
	gtk_main_quit();  
}



/* keyborad callback function */  
gboolean on_main_window_key_press_event (GtkWidget *widget,GdkEventKey *event,gpointer user_data)
{
	g_print("on_main_window_key_press_event\n"); 
	SDL_Event sdlevent;
    switch(event->keyval) {
    case GDK_Up:
        g_print("Up\n");
		//ffplay forward 
		sdlevent.type = SDL_KEYDOWN;
		sdlevent.key.keysym.sym = SDLK_RIGHT;
		SDL_PushEvent(&sdlevent);
        break;
    case GDK_Left:
        g_print("Left\n");
		//ffplay rewind
		sdlevent.type = SDL_KEYDOWN;
		sdlevent.key.keysym.sym = SDLK_LEFT;
		SDL_PushEvent(&sdlevent);
        break;
    case GDK_Right:
        g_print("Right\n");
		//ffplay forward
		sdlevent.type = SDL_KEYDOWN;
		sdlevent.key.keysym.sym = SDLK_RIGHT;
		SDL_PushEvent(&sdlevent);
        break;
    case GDK_Down:
        g_print("Down\n");
		//ffplay rewind
		sdlevent.type = SDL_KEYDOWN;
		sdlevent.key.keysym.sym = SDLK_LEFT;
		SDL_PushEvent(&sdlevent);
        break;
	case GDK_Escape:
		g_print("Esc\n");
		//ffplay exit
		sdlevent.type = FF_QUIT_EVENT;
		SDL_PushEvent(&sdlevent);
        break;
	case GDK_space:
		g_print("GDK_space\n");
        break;
	default:
		break;
    }
      return FALSE;
}

/* Destroy window*/  
gint delete_event( GtkWidget *widget,GdkEvent *event,gpointer data )
{
	gtk_main_quit ();
	return FALSE;
} 

/* Load video file*/  
gboolean load_file(gchar *uri)  
{ 
	g_print("load_file\n");  
	
	char fn[256],*p;
	char pathname[256];
	int err;
	
	//创建ffplay播放视频线程
    err = pthread_create(&playeropen_msg_process_thread_tid, NULL, (void *)playeropen_thread, uri);
    if (err != 0)
        printf("can't create thread: %s\n", strerror(err));
	else
		printf("playeropen_thread pthread_create success\n");
	
	//父线程调用pthread_detach(thread_id)（非阻塞，可立即返回）
	//这将该子线程的状态设置为分离的（detached），如此一来，该线程运行结束后会自动释放所有资源。
	pthread_detach(playeropen_msg_process_thread_tid);
	
	//从路径名中分离文件名
	//全文件名赋值给pathname
	strncpy(pathname, uri,256);

	//第2实参这样写以防止文件在当前目录下时因p=NULL而出错
	strcpy(fn,(p=strrchr(pathname,'/')) ? p+1 : pathname);
	
	//打出来看看
	printf("%s\n",fn);
	
	//更新窗口标题为视频文件名 
    //gtk_window_set_title(GTK_WINDOW(main_window),(char *)fn);  
	//gtk_label_set_text(GTK_LABEL(video_title_label),(char *)fn);
	
	/* Connect a callback to trigger every 200 milliseconds to 
	* update the GUI with the playback progress. We remember 
	* the ID of this source so that we can remove it when we stop 
	* playing */  
	timeout_source = g_timeout_add(500, (GSourceFunc)update_time_callback, NULL);  
		
	//显示  
    gtk_widget_show_all(GTK_WIDGET(main_window));   
	
	return TRUE;
}  

/* Create GUI interface*/  
GtkWidget *build_gui()  
{  

    // 创建主 GtkVBOx. 其他所有都在它里面  
    // 0：各个构件高度可能不同，0：构件之间的间距为0像素  
    main_vbox = gtk_vbox_new(0, 0);  
  	
	// status_controls_hbox :close button
    status_controls_hbox = gtk_hbox_new(FALSE, 0);  
	gtk_widget_set_size_request(GTK_WIDGET(status_controls_hbox),-1,STATUS_CONTROLS_HBOX_HEIGHT);
    gtk_box_pack_start(GTK_BOX(main_vbox), status_controls_hbox, FALSE, FALSE, 0);  
	
	//关闭按钮
    close_button = gtk_button_new();  
	//GtkWidget* img_close = gtk_image_new_from_stock(GTK_STOCK_CLOSE,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_close= gtk_image_new_from_file("/usr/share/ffplay/close.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(close_button),img_close);
	//设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件 TRUE表示响应鼠标键盘事件
    gtk_widget_set_sensitive(GTK_WIDGET(close_button), FALSE); 
    g_signal_connect(G_OBJECT(close_button), "clicked", G_CALLBACK(toggle_close_button_callback), NULL);  
	gtk_box_pack_end(GTK_BOX(status_controls_hbox), close_button, FALSE, FALSE, 0);
  

	// play_controls_hbox  
    play_controls_hbox = gtk_hbox_new(FALSE, 10);  
	gtk_widget_set_size_request(GTK_WIDGET(play_controls_hbox),-1,PLAY_CONTROLS_HBOX_HEIGHT);
    //gtk_box_pack_start(GTK_BOX(main_vbox), play_controls_hbox, FALSE, FALSE, 0);  
	gtk_box_pack_end(GTK_BOX(main_vbox), play_controls_hbox, FALSE, FALSE, 0);  

	//播放/暂停按钮
    play_button = gtk_button_new();  
	//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_play= gtk_image_new_from_file("/usr/share/ffplay/play.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(play_button),img_play);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件 TRUE表示响应鼠标键盘事件
    gtk_widget_set_sensitive(play_button, TRUE);
	
    g_signal_connect(G_OBJECT(play_button), "clicked", G_CALLBACK(toggle_play_pause_button_callback), NULL);  
	//gtk_widget_set_size_request (GTK_WIDGET(play_button), 30, 30);
	gtk_box_pack_start(GTK_BOX(play_controls_hbox), play_button, FALSE, FALSE, 10);
	
	//正在播放的时间标签     
    play_time_label = gtk_label_new("00:00:00");  
    //gtk_misc_set_alignment(GTK_MISC(play_time_label), 0.0, 0.5);  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), play_time_label, FALSE, FALSE, 0);  
 
    // 视频进度条控制  
	/* value, lower, upper, step_increment, page_increment, page_size */
	/* 注意，page_size值只对滚动条构件有区别，并且，你实际上能取得的最高值是(upper - page_size)。 */
	//video_schedule_adj = gtk_adjustment_new (0.0, 0.0, 101.0, 0.1, 1.0, 1.0);
	video_schedule_adj = gtk_adjustment_new (0.00, 0.00, 101.00, 0.01, 0.1, 1.0);
	//video_schedule_adj = gtk_adjustment_new (0, 0, 101, 1, 1, 1);
	seek_scale = gtk_hscale_new (GTK_ADJUSTMENT (video_schedule_adj));
    gtk_scale_set_draw_value(GTK_SCALE(seek_scale), FALSE); 
	gtk_scale_set_digits(GTK_SCALE(seek_scale),2);	
    //gtk_range_set_update_policy(GTK_RANGE(seek_scale), GTK_UPDATE_DISCONTINUOUS);  
	gtk_range_set_update_policy(GTK_RANGE(seek_scale), GTK_UPDATE_CONTINUOUS);  
	gtk_scale_set_value_pos (GTK_SCALE(seek_scale), GTK_POS_LEFT);
	gtk_range_set_adjustment(GTK_RANGE(seek_scale),GTK_ADJUSTMENT(video_schedule_adj));
    g_signal_connect(G_OBJECT(seek_scale), "value-changed", G_CALLBACK(video_seek_value_changed), NULL);  
	//其他控件全部按照构件自身大小布局，剩余空间全部充满进度条构件
	 gtk_box_pack_start(GTK_BOX(play_controls_hbox), seek_scale, TRUE, TRUE, 0);  
		
	//总播放的时间标签     
    total_time_label = gtk_label_new("00:00:00");  
    //gtk_misc_set_alignment(GTK_MISC(time_label), 0.0, 0.5);  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), total_time_label, FALSE, FALSE, 0);  
	
	//静音按钮
    voice_slience_button = gtk_button_new();  
	//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_voice= gtk_image_new_from_file("/usr/share/ffplay/voice.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(voice_slience_button),img_voice);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(voice_slience_button, FALSE);
    g_signal_connect(G_OBJECT(voice_slience_button), "clicked", G_CALLBACK(toggle_voice_slience_button_callback), NULL);  
	gtk_box_pack_start(GTK_BOX(play_controls_hbox), voice_slience_button, FALSE, FALSE, 0);


	// 音量进度条控制  
	/* value, lower, upper, step_increment, page_increment, page_size */
	/* 注意，page_size值只对滚动条构件有区别，并且，你实际上能取得的最高值是(upper - page_size)。 */
	/*音量的可调整范围是0-128*/
	voice_schedule_adj = gtk_adjustment_new (128, 0, 129, 1, 1, 1);
	voice_scale = gtk_hscale_new (GTK_ADJUSTMENT (voice_schedule_adj));
	gtk_widget_set_size_request (GTK_WIDGET(voice_scale),129,-1);
    gtk_scale_set_draw_value(GTK_SCALE(voice_scale), FALSE);  
	gtk_scale_set_digits(GTK_SCALE(voice_scale),0);
    gtk_range_set_update_policy(GTK_RANGE(voice_scale), GTK_UPDATE_CONTINUOUS);  
	gtk_scale_set_value_pos (GTK_SCALE(voice_scale), GTK_POS_LEFT);
	gtk_range_set_adjustment(GTK_RANGE(voice_scale),GTK_ADJUSTMENT(voice_schedule_adj));
    g_signal_connect(G_OBJECT(voice_scale), "value-changed", G_CALLBACK(voice_seek_value_changed), NULL);  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), voice_scale, FALSE, FALSE, 0);  
	
	//空白标签：目的是分隔音量条和窗口边界为10个像素点   
    blank_space_label = gtk_label_new(" ");  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), blank_space_label, FALSE, FALSE, 0);  
	
	
	// Get the size of status_controls_hbox
	gtk_widget_get_size_request (GTK_WIDGET(status_controls_hbox), &status_controls_hbox_width, &status_controls_hbox_height);
	printf("status_controls_hbox:width %d, height %d\n",status_controls_hbox_width,status_controls_hbox_height); 
	
	// Get the size of play_controls_hbox
	gtk_widget_get_size_request (GTK_WIDGET(play_controls_hbox), &play_controls_hbox_width, &play_controls_hbox_height);
	printf("play_controls_hbox:width %d, height %d\n",play_controls_hbox_width,play_controls_hbox_height);  
 
	
	
	// 视频显示区域 
	video_output = gtk_drawing_area_new (); 
	gtk_widget_set_size_request (GTK_WIDGET(video_output), gtk_screen_width, (gtk_screen_height-( play_controls_hbox_height + status_controls_hbox_height)));
	//turn off gtk double buffer!!
	gtk_widget_set_double_buffered(video_output, FALSE);
	gtk_box_pack_end (GTK_BOX (main_vbox), video_output, TRUE, TRUE, 0); 
	
	// Get the size of video_output
	gtk_widget_get_size_request (GTK_WIDGET(video_output), &video_output_width, &video_output_height);
	printf("video_output:width %d, height %d\n",video_output_width,video_output_height);  
	set_video_output_height(video_output_height);

    return main_vbox;  
}  	 

int main(int argc, char *argv[])  
{  

	int i=0;
	char *filename=NULL;  
 
    // 初始化 GTK+  
    gtk_init(&argc, &argv);  
	
	//load file to play by user's cmdline parameter
	printf("You have inputed total %d argments\n",argc);  
	for(i=0;i<argc;i++)  
	{  
		printf("arg%d : %s\n",i,argv[i]);  
	} 
	filename = argv[1];  
	if (!filename) 
	{
		show_usage();
		av_log(NULL, AV_LOG_FATAL, "An input file must be specified\n");
		av_log(NULL, AV_LOG_FATAL,
		"Use -h to get full help or, even better, run 'man %s'\n", program_name);
		exit(1);
	}
	if (current_filename) g_free(current_filename);  
	current_filename = filename;  
	
    /* Get the Screen Resolution */
    gtk_screen = gdk_screen_get_default();
    gtk_screen_width = gdk_screen_get_width(gtk_screen);
    gtk_screen_height = gdk_screen_get_height(gtk_screen);
    printf("gtk_screen gtk_screen_width: %d, gtk_screen_height: %d\n", gtk_screen_width, gtk_screen_height);
  
    //创建窗口  
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
	//设置窗口居中显示
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);  	
    //设置窗口标题  
    gtk_window_set_title(GTK_WINDOW(main_window), "FFMPEG Player");  
	//设置窗口大小,整个窗口设置成分辨率大小 
	gtk_window_set_default_size(GTK_WINDOW(main_window), gtk_screen_width, gtk_screen_height); 
	
	/* hide the title bar and the boder */ 
	gtk_window_set_decorated(GTK_WINDOW(main_window), FALSE); 	
	
	
	/* 你应该总是记住连接 delete_event 信号到主窗口。这对适当的直觉行为很重要 */
	g_signal_connect (G_OBJECT (main_window), "delete_event",G_CALLBACK (delete_event), NULL);
	
	//键盘获取事件
    g_signal_connect(G_OBJECT(main_window), "key-press-event", G_CALLBACK(on_main_window_key_press_event), NULL);
	
    // 创建主窗口GUI  
    gtk_container_add(GTK_CONTAINER(main_window), build_gui());  
	
	
	 //bind Accelerate for play_button 
	play_button_accelerate = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(main_window), play_button_accelerate);
	//快捷键注册，其实就是当快捷键按下的时候，为控件触发一个信号
	//(GdkModifierType)0为不使用修饰键
	gtk_widget_add_accelerator(play_button,"clicked",play_button_accelerate,GDK_space,(GdkModifierType)0,GTK_ACCEL_VISIBLE);

    //显示  
    gtk_widget_show_all(GTK_WIDGET(main_window));   
	
	if(1 == gtk_widget_is_focus(play_button)) 
	{
		printf("1 == gtk_widget_is_focus(play_button)\n");  
	}
	if(1 == gtk_widget_is_focus(close_button)) 
	{
		printf("1 == gtk_widget_is_focus(close_button)\n");  
	}
	
    //主界面绘制完成后，用户不可以调整窗口大小
	gtk_window_set_resizable (GTK_WINDOW (main_window), FALSE);	

	//g_thread_init (NULL);
	
	//SDL播放画面嵌入到GTK窗口,切记务必要在整个界面show之后再去获取GTK窗口ID
	//在GtkWidget没有show出来之前是没有XID（window ID）的。
	char SDL_windowhack[32];
	//获取GTK视频显示窗口ID,并格式化字符串SDL_windowhack 
	sprintf(SDL_windowhack, "SDL_WINDOWID=%ld", GDK_WINDOW_XID(gtk_widget_get_window(video_output)));
	g_print("SDL_WINDOWID:=0x%1x\n",(unsigned int)GDK_WINDOW_XID(gtk_widget_get_window(video_output))); 
	//设置SDL显示窗口环境变量
	putenv(SDL_windowhack);
	

	//load file 
	if (load_file(current_filename))  
	{
		gtk_widget_set_sensitive(GTK_WIDGET(play_button), TRUE); 
		gtk_widget_set_sensitive(GTK_WIDGET(voice_slience_button), TRUE);  	
	    gtk_widget_set_sensitive(GTK_WIDGET(close_button), TRUE);  	
	}
	
    //开始主循环  
    gtk_main();  
  
    return 0;  
}  


