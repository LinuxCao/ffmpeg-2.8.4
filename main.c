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

  
/************************************************************************
* Static Variable Define Section
************************************************************************/
static GtkWidget *main_window;  
static GtkWidget *total_time_label;  //总播放的时间标签
static GtkWidget *play_time_label;  //正在播放的时间标签
static GtkWidget *separated_time_label; //时间标签分隔符 
static GtkWidget *seek_scale;  
static GtkWidget *video_output; 
static GtkWidget *voice_scale;  

static GtkWidget *play_button;   //播放暂停按钮
static GtkWidget *rewind_button;  //快退按钮
static GtkWidget *forward_button; //快进按钮
static GtkWidget *fullscreen_button; //全屏按钮
static GtkWidget *voice_slience_button; //静音按钮
GtkObject *video_schedule_adj;//video  schedule adjustment
GtkObject *voice_schedule_adj;//voice  schedule adjustment

#define XSIZE 1280
#define YSIZE 720

static char *current_filename = NULL;  
static char total_time_label_string[32]={0};
static char play_time_label_string[32]={0};
static guint timeout_source = 0;   
pthread_t playeropen_msg_process_thread_tid; 				//视频消息处理线程

gboolean seek_flag = FALSE;  

/************************************************************************
* Function Define Section
************************************************************************/


 // 函数实现
void *playeropen_thread(char *file)
{
	g_print("playeropen_thread start\n"); 
	char *pfile[2];
	pfile[0]="ffplay";
	pfile[1]= file;
	printf("file:%s\n",file);
	printf("pfile[0]:%s\n",pfile[0]);
	printf("pfile[1]:%s\n",pfile[1]);
	//playeropen_msg_process_thread
	ffplay_init(2,pfile);
	
}

// 打开文件  
static void file_open(GtkAction *action)  
{  
	g_print("file_open\n"); 
	GtkWidget *file_chooser = gtk_file_chooser_dialog_new(  
        "Open File", GTK_WINDOW(main_window),  
        GTK_FILE_CHOOSER_ACTION_OPEN,  
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,  
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,  
        NULL);  
  
    if (gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT) 
	{  
		char *filename;  
		filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(file_chooser));  
		// g_signal_emit_by_name(G_OBJECT(stop_button), "clicked");  
		if (current_filename) g_free(current_filename);  
		current_filename = filename;  
		//load file 
        if (load_file(filename))  
		{
            gtk_widget_set_sensitive(GTK_WIDGET(play_button), TRUE); 
			gtk_widget_set_sensitive(GTK_WIDGET(rewind_button), TRUE); 
			gtk_widget_set_sensitive(GTK_WIDGET(forward_button), TRUE); 
			gtk_widget_set_sensitive(GTK_WIDGET(fullscreen_button), TRUE); 
			gtk_widget_set_sensitive(GTK_WIDGET(voice_slience_button), TRUE);  			

		}
	}
	gtk_widget_destroy(file_chooser);
}  
// 退出  
static void file_quit(GtkAction *action)  
{  
    gtk_main_quit();  
}  
// 关于  
static void help_about(GtkAction *action)  
{  
	g_print("help_about\n"); 
	GtkWidget *about_dialog = gtk_about_dialog_new();  
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about_dialog), "FFMPEG Player");  
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "0.0.0");  
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), "Copyright @ 2015, OS-easy");  

	gtk_dialog_run(GTK_DIALOG(about_dialog));  
	gtk_widget_destroy(about_dialog);  
}  
  
static GtkActionEntry mainwindow_action_entries[] = {  
    { "FileMenu", "NULL", "文件" },  
    {  
        "OpenFile",  
        GTK_STOCK_OPEN,  
        "打开(O)",  
        "<control>O",  
        "Open a file for playback",  
        G_CALLBACK(file_open)  
    },  
    {  
        "QuitPlayer",  
        GTK_STOCK_QUIT,  
        "退出(Q)",  
        "<control>Q",  
        "Quit the media player",  
        G_CALLBACK(file_quit)  
    },  
    
    { "HelpMenu", "NULL", "帮助" },  
    {  
        "HelpAbout",  
        GTK_STOCK_ABOUT,  
        "关于",  
        "",  
        "About the media player",  
        G_CALLBACK(help_about)  
    }  
};  

void toggle_fullscreen_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_fullscreen_button_callback\n"); 
	SDL_Event sdlevent;
	sdlevent.type = SDL_VIDEORESIZE;
	//设置全屏的尺寸
	sdlevent.resize.w=get_fs_screen_width();
	sdlevent.resize.h=get_fs_screen_height()-200;
	//sdlevent.resize.h=YSIZE;
	//gtk_widget_set_size_request (GTK_WIDGET (video_output), get_fs_screen_width(), YSIZE);
	gtk_widget_set_size_request (GTK_WIDGET (video_output), get_fs_screen_width(), get_fs_screen_height()-200);
	SDL_PushEvent(&sdlevent);
	gtk_widget_show(GTK_WIDGET (video_output));
	//用户不可用调整窗口大小
    gtk_window_set_resizable (GTK_WINDOW (main_window), FALSE);
}

/* Handler for user moving seek bar */  
static void video_seek_value_changed(GtkRange *range, gpointer data)  
{  

	if(get_videostate_for_gtk() && seek_flag==TRUE)
	{
		g_print("video_seek_value_changed\n");  
		int64_t ts;
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		double current_x=0.0,frac=0.0;
		VideoState* cur_stream;
		
		current_x= gtk_adjustment_get_value(GTK_ADJUSTMENT (video_schedule_adj));
		cur_stream=get_videostate_for_gtk();
		
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

//更新播放时间回调函数
gboolean update_time_callback()  
{  
	seek_flag=FALSE;
 	//g_print("update_time_callback\n");   
	if(get_videostate_for_gtk())
	{
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		VideoState* cur_stream;
		double frac=0;
		double current_x=0.0;
	
		//获取总的播放时间
		cur_stream=get_videostate_for_gtk();
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

	}
	else
	{
		g_print("Failed to initialize VideoState!\n");
	}
	seek_flag=TRUE;
	return TRUE;
}  
  
/* Handler for user moving voice_value bar */  
static void voice_seek_value_changed(GtkRange *range, gpointer data)  
{  
	g_print("voice_seek_value_changed\n");  
	double volume_value = gtk_adjustment_get_value(GTK_ADJUSTMENT (voice_schedule_adj));
	if(volume_value >= 0 && volume_value <= 128)
	{
		g_print("set_default_volume_value:%2f\n",volume_value);  
		set_default_volume_value(volume_value);
	}
}  
void toggle_play_pause_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_play_pause_button_callback\n"); 
	if(current_filename)
	{
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))//play
		{
			g_print("GTK_STOCK_MEDIA_PLAY\n");   
			//使用内置的图标创建图像
			//GtkWidget* img_play = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
			//使用指定图标创建按钮图像
			GtkWidget* img_play= gtk_image_new_from_file("play.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(widget),img_play);
			gtk_widget_show(widget);
			
			//ffplay pause
			SDL_Event sdlevent;
			sdlevent.type = SDL_KEYDOWN;
			sdlevent.key.keysym.sym = SDLK_SPACE;
			SDL_PushEvent(&sdlevent);
			
			//refresh fullscreen
			//toggle_fullscreen_button_callback (GTK_WIDGET (video_output), NULL);
			
		} 
		else //pause
		{
			g_print("GTK_STOCK_MEDIA_PAUSE\n");   
			//使用内置的图标创建图像
			//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,GTK_ICON_SIZE_BUTTON);
			//使用指定图标创建按钮图像
			GtkWidget* img_pause= gtk_image_new_from_file("pause.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(widget),img_pause);
			gtk_widget_show(widget);

			//ffplay play
			SDL_Event sdlevent;
			sdlevent.type = SDL_KEYDOWN;
			sdlevent.key.keysym.sym = SDLK_SPACE;
			SDL_PushEvent(&sdlevent);
			
			//refresh fullscreen
			//toggle_fullscreen_button_callback (GTK_WIDGET (video_output), NULL);
		}
	}
	else
	{
		g_print("please choose open video file.\n"); 
	}
}

void toggle_rewind_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_rewind_button_callback\n"); 
	//快退 10 秒
	SDL_Event sdlevent;
	sdlevent.type = SDL_KEYDOWN;
	sdlevent.key.keysym.sym = SDLK_LEFT;
	SDL_PushEvent(&sdlevent);
}


void toggle_forward_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_forward_button_callback\n"); 
	//快进 10 秒
	SDL_Event sdlevent;
	sdlevent.type = SDL_KEYDOWN;
	sdlevent.key.keysym.sym = SDLK_RIGHT;
	SDL_PushEvent(&sdlevent);
}



void toggle_voice_slience_button_callback (GtkWidget *widget, gpointer data)
{
	g_print("toggle_voice_slience_button_callback\n"); 
	if(current_filename)
	{
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))//voice
		{
			g_print("voice\n");   
			//使用内置的图标创建图像
			//GtkWidget* img_play = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
			//使用指定图标创建按钮图像
			GtkWidget* img_voice= gtk_image_new_from_file("voice.png");
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

			

		} 
		else //slience
		{
			g_print("slience\n");   
			//使用内置的图标创建图像
			//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,GTK_ICON_SIZE_BUTTON);
			//使用指定图标创建按钮图像
			GtkWidget* img_slience= gtk_image_new_from_file("slience.png");
			//动态设置按钮的图像
			gtk_button_set_image(GTK_BUTTON(widget),img_slience);
			gtk_widget_show(widget);
			
			//Slience SDL Audio
			g_print("Slience SDL Audio :set_default_volume_value(0)\n");
			set_default_volume_value(0);
			


		}
	}
	else
	{
		g_print("please choose open video file.\n"); 
	}
}

GtkWidget *build_gui()  
{  
    GtkWidget *main_vbox;  
    GtkWidget *voice_status_hbox;  
	GtkWidget *play_controls_hbox;   
    GtkWidget *status_controls_hbox;
	
	/* Get the Screen Resolution */
	GdkScreen* screen;
    gint width, height;
    screen = gdk_screen_get_default();
    width = gdk_screen_get_width(screen);
    height = gdk_screen_get_height(screen);
    printf("screen width: %d, height: %d\n", width, height);
  
    GtkActionGroup *actiongroup;  
    GtkUIManager *ui_manager;  
  
    actiongroup = gtk_action_group_new("MainwindowActiongroup");  
    gtk_action_group_add_actions(actiongroup,  
        mainwindow_action_entries,  
        G_N_ELEMENTS(mainwindow_action_entries),  
        NULL);  
  
    ui_manager = gtk_ui_manager_new();  
    gtk_ui_manager_insert_action_group(ui_manager, actiongroup, 0);  
    gtk_ui_manager_add_ui_from_string(  
        ui_manager,  
        "<ui>"  
        "    <menubar name='MainMenu'>"  
        "        <menu action='FileMenu'>"  
        "            <menuitem action='OpenFile'/>"  
        "            <separator name='fsep1'/>"  
        "            <menuitem action='QuitPlayer'/>"  
        "        </menu>"  
        "        <menu action='HelpMenu'>"  
        "            <menuitem action='HelpAbout'/>"  
        "        </menu>"         
        "    </menubar>"  
        "</ui>",  
        -1,  
        NULL);  
      
  
    // 创建主 GtkVBOx. 其他所有都在它里面  
    // 0：各个构件高度可能不同，6：构件之间的间距为6 像素  
    main_vbox = gtk_vbox_new(0, 6);  
  
    // 添加菜单栏  
    //gtk_box_pack_start(GTK_BOX(main_vbox),gtk_ui_manager_get_widget(ui_manager, "/ui/MainMenu"),FALSE, FALSE, 0);  
  
    
    // 视频显示区域 
    video_output = gtk_drawing_area_new (); 
	gtk_widget_set_size_request (GTK_WIDGET(video_output), width, (height-200));
    gtk_box_pack_start (GTK_BOX (main_vbox), video_output, TRUE, TRUE, 0); 

 
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
    gtk_box_pack_start(GTK_BOX(main_vbox), seek_scale, FALSE, FALSE, 0);  
	
	// status_controls_hbox  
    status_controls_hbox = gtk_hbox_new(FALSE, 10);  
    gtk_box_pack_start(GTK_BOX(main_vbox), status_controls_hbox, FALSE, FALSE, 0);  

	// play_controls_hbox  
    play_controls_hbox = gtk_hbox_new(FALSE, 10);  
    gtk_box_pack_start(GTK_BOX(status_controls_hbox), play_controls_hbox, FALSE, FALSE, 0);  
	
	//正在播放的时间标签     
    play_time_label = gtk_label_new("00:00:00");  
    gtk_misc_set_alignment(GTK_MISC(play_time_label), 0.0, 0.5);  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), play_time_label, FALSE, FALSE, 0);  
	
	//时间标签分隔符     
    separated_time_label = gtk_label_new("/");  
    //gtk_misc_set_alignment(GTK_MISC(time_label), 0.0, 0.5);  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), separated_time_label, FALSE, FALSE, 0);  
	
	//总播放的时间标签     
    total_time_label = gtk_label_new("00:00:00");  
    //gtk_misc_set_alignment(GTK_MISC(time_label), 0.0, 0.5);  
    gtk_box_pack_start(GTK_BOX(play_controls_hbox), total_time_label, FALSE, FALSE, 0);  
	
	//快退按钮
    rewind_button = gtk_toggle_button_new();  
	//GtkWidget* img_rewind = gtk_image_new_from_stock(GTK_STOCK_MEDIA_REWIND ,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_rewind = gtk_image_new_from_file("rewind.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(rewind_button),img_rewind);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(rewind_button, FALSE);
	//默认是激活状态
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rewind_button),TRUE);
    g_signal_connect(G_OBJECT(rewind_button), "clicked", G_CALLBACK(toggle_rewind_button_callback), NULL);  
	gtk_box_pack_start(GTK_BOX(play_controls_hbox), rewind_button, FALSE, FALSE, 0);
	
	//播放/暂停按钮
    play_button = gtk_toggle_button_new();  
	//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_play= gtk_image_new_from_file("play.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(play_button),img_play);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(play_button, FALSE);
	//默认是处于播放toggle,用户再点一下就是暂停toggle
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(play_button),TRUE);
    g_signal_connect(G_OBJECT(play_button), "clicked", G_CALLBACK(toggle_play_pause_button_callback), NULL);  
	gtk_box_pack_start(GTK_BOX(play_controls_hbox), play_button, FALSE, FALSE, 0);
	
	//快进按钮
    forward_button = gtk_toggle_button_new();  
	//GtkWidget* img_forward = gtk_image_new_from_stock( GTK_STOCK_MEDIA_FORWARD ,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_forward = gtk_image_new_from_file("forward.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(forward_button),img_forward);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(forward_button, FALSE);
	//默认是激活状态
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(forward_button),TRUE);
    g_signal_connect(G_OBJECT(forward_button), "clicked", G_CALLBACK(toggle_forward_button_callback), NULL);  
	gtk_box_pack_start(GTK_BOX(play_controls_hbox), forward_button, FALSE, FALSE, 0);
	
	// voice_status_hbox  
    voice_status_hbox = gtk_hbox_new(FALSE, 10);  
    gtk_box_pack_end(GTK_BOX(status_controls_hbox), voice_status_hbox, FALSE, FALSE, 0);  
	
	//静音按钮
    voice_slience_button = gtk_toggle_button_new();  
	//GtkWidget* img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_BUTTON);
	GtkWidget* img_voice= gtk_image_new_from_file("voice.png");
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(voice_slience_button),img_voice);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(voice_slience_button, FALSE);
	//默认是处于音量toggle,用户再点一下就是静音
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(voice_slience_button),TRUE);
    g_signal_connect(G_OBJECT(voice_slience_button), "clicked", G_CALLBACK(toggle_voice_slience_button_callback), NULL);  
	gtk_box_pack_start(GTK_BOX(voice_status_hbox), voice_slience_button, FALSE, FALSE, 0);
	
	// 音量进度条控制  
	/* value, lower, upper, step_increment, page_increment, page_size */
	/* 注意，page_size值只对滚动条构件有区别，并且，你实际上能取得的最高值是(upper - page_size)。 */
	/*音量的可调整范围是0-128*/
	voice_schedule_adj = gtk_adjustment_new (128, 0, 129, 1, 1, 1);
	voice_scale = gtk_hscale_new (GTK_ADJUSTMENT (voice_schedule_adj));
	gtk_widget_set_size_request (GTK_WIDGET(voice_scale),200,-1);
    gtk_scale_set_draw_value(GTK_SCALE(voice_scale), TRUE);  
	gtk_scale_set_digits(GTK_SCALE(voice_scale),0);
    gtk_range_set_update_policy(GTK_RANGE(voice_scale), GTK_UPDATE_CONTINUOUS);  
	gtk_scale_set_value_pos (GTK_SCALE(voice_scale), GTK_POS_LEFT);
	gtk_range_set_adjustment(GTK_RANGE(voice_scale),GTK_ADJUSTMENT(voice_schedule_adj));
    g_signal_connect(G_OBJECT(voice_scale), "value-changed", G_CALLBACK(voice_seek_value_changed), NULL);  
    gtk_box_pack_start(GTK_BOX(voice_status_hbox), voice_scale, FALSE, FALSE, 0);  
	
	
	
	//全屏按钮
    fullscreen_button = gtk_toggle_button_new();  
	GtkWidget* img_fullscreen = gtk_image_new_from_stock( GTK_STOCK_ZOOM_FIT ,GTK_ICON_SIZE_BUTTON);
	//动态设置按钮的图像
	gtk_button_set_image(GTK_BUTTON(fullscreen_button),img_fullscreen);
    //设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(fullscreen_button, FALSE);
	//默认是处于全屏toggle,用户再点一下就是1:1播放
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fullscreen_button),TRUE);
    g_signal_connect(G_OBJECT(fullscreen_button), "clicked", G_CALLBACK(toggle_fullscreen_button_callback), NULL);  
	gtk_box_pack_end(GTK_BOX(voice_status_hbox), fullscreen_button, FALSE, FALSE, 0);

    return main_vbox;  
}  
  
  
// destory main window  
static void destroy(GtkWidget *widget, gpointer data)  
{  
    gtk_main_quit();  
}  
 


// load file to play  
gboolean load_file(gchar *uri)  
{ 
	g_print("load_file\n");  
	
	char fn[256],*p;
	char pathname[256];
	int err;
	
	//创建ffplay播放视频线程
    err = pthread_create(&playeropen_msg_process_thread_tid, NULL, playeropen_thread, uri);
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
    gtk_window_set_title(GTK_WINDOW(main_window),(char *)fn);  
	
	/* Connect a callback to trigger every 200 milliseconds to 
	* update the GUI with the playback progress. We remember 
	* the ID of this source so that we can remove it when we stop 
	* playing */  
	timeout_source = g_timeout_add(500, (GSourceFunc)update_time_callback, NULL);  
		
	//显示  
    gtk_widget_show_all(GTK_WIDGET(main_window));   
	
	return TRUE;
}  

  
int main(int argc, char *argv[])  
{  

	int i=0;
	char *filename=NULL;  
 
    // 初始化 GTK+  
    gtk_init(&argc, &argv);  
    //gst_init(&argc, &argv);  
    // 创建窗口  
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    // 设置窗口标题  
    gtk_window_set_title(GTK_WINDOW(main_window), "FFMPEG Player");  
	
	//用户可以自动调整窗口大小
	gtk_window_set_resizable (GTK_WINDOW (main_window), TRUE);
    // 主窗口销毁句柄  
    g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(destroy), NULL);  
    // 创建主窗口GUI  
    gtk_container_add(GTK_CONTAINER(main_window), build_gui());  
	

    // 显示  
    gtk_widget_show_all(GTK_WIDGET(main_window));   

    //主界面绘制完成后，用户不可以调整窗口大小
	gtk_window_set_resizable (GTK_WINDOW (main_window), FALSE);	

	g_thread_init (NULL);
	
	//SDL播放画面嵌入到GTK窗口,切记务必要在整个界面show之后再去获取GTK窗口ID
	//在GtkWidget没有show出来之前是没有XID（window ID）的。
	char SDL_windowhack[32];
	//获取GTK视频显示窗口ID,并格式化字符串SDL_windowhack 
	sprintf(SDL_windowhack, "SDL_WINDOWID=%ld", GDK_WINDOW_XID(video_output->window));
	g_print("SDL_WINDOWID:=0x%1x\n",(unsigned int)GDK_WINDOW_XID(video_output->window)); 
	//设置SDL显示窗口环境变量
	putenv(SDL_windowhack);
	
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
	//load file 
	if (load_file(filename))  
	{
		gtk_widget_set_sensitive(GTK_WIDGET(play_button), TRUE); 
		gtk_widget_set_sensitive(GTK_WIDGET(rewind_button), TRUE); 
		gtk_widget_set_sensitive(GTK_WIDGET(forward_button), TRUE); 
		gtk_widget_set_sensitive(GTK_WIDGET(fullscreen_button), TRUE); 
		gtk_widget_set_sensitive(GTK_WIDGET(voice_slience_button), TRUE);  			
	}
	
    // 开始主循环  
    gtk_main();  
  
    return 0;  
}  


