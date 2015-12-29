/* 
*main.c  
*Simple media player based on Gstreamer and GTK 
*/   
#include <gdk/gdkx.h>  
#include <gtk/gtk.h>  
#include <string.h>  
#include <stdio.h>

#include "main.h"  
#include "ffplay.h" 

  
static GtkWidget *main_window;  
static GtkWidget *play_button;  
static GtkWidget *pause_button;  
static GtkWidget *stop_button;  
static GtkWidget *status_label;  
static GtkWidget *time_label;  
static GtkWidget *seek_scale;  
static GtkWidget *video_output;  

static char *current_filename = NULL;  
 
 // 函数实现
void* playeropen(const gchar *file)
{
	g_print("playeropen\n"); 
	char *pfile[2];
	pfile[0]="ffplay";
	pfile[1]= file;
	printf("file:%s\n",file);
	printf("pfile[0]:%s\n",pfile[0]);
	printf("pfile[1]:%s\n",pfile[1]);
	ffplay_init(2,pfile);
	return TRUE;

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
  
static void play_clicked(GtkWidget *widget, gpointer data)  
{  
	g_print("play_clicked\n"); 

	if (current_filename) 
	{  
		if (play_file())
		{  
			gtk_widget_set_sensitive(GTK_WIDGET(stop_button), TRUE);  
			gtk_widget_set_sensitive(GTK_WIDGET(pause_button), TRUE);  
			gtk_widget_set_sensitive(GTK_WIDGET(play_button), FALSE); 
			g_print("gui_status_update(STATE_PLAY)\n");    
			gui_status_update(STATE_PLAY);  
			g_print("Play success\n");    
		}  
		else 
		{  
			g_print("Failed to play\n");  
		}  
	}  
}  
  
static void pause_clicked(GtkWidget *widget, gpointer data)  
{           
	g_print("pause_clicked\n");               
}  
  
static void stop_clicked(GtkWidget *widget, gpointer data)  
{        
	g_print("stop_clicked\n");   
}  
  
/* Handler for user moving seek bar */  
static void seek_value_changed(GtkRange *range, gpointer data)  
{  
	g_print("seek_value_changed\n");     
}  
  
GtkWidget *build_gui()  
{  
    GtkWidget *main_vbox;  
    GtkWidget *status_hbox;  
    GtkWidget *controls_hbox;  
   // GtkWidget *saturation_controls_hbox;  
  
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
    main_vbox = gtk_vbox_new(0, 0);  
  
    // 添加菜单栏  
    gtk_box_pack_start(  
        GTK_BOX(main_vbox),  
        gtk_ui_manager_get_widget(ui_manager, "/ui/MainMenu"),  
        FALSE, FALSE, 0);  
  
    
    // 视频显示区域 
    video_output = gtk_drawing_area_new (); 
    gtk_box_pack_start (GTK_BOX (main_vbox), video_output, TRUE, TRUE, 0); 

 
    // 滑动条控制  
    seek_scale = gtk_hscale_new_with_range(0, 100, 1);  
    gtk_scale_set_draw_value(GTK_SCALE(seek_scale), FALSE);  
    gtk_range_set_update_policy(GTK_RANGE(seek_scale), GTK_UPDATE_DISCONTINUOUS);  
    g_signal_connect(G_OBJECT(seek_scale), "value-changed", G_CALLBACK(seek_value_changed), NULL);  
    gtk_box_pack_start(GTK_BOX(main_vbox), seek_scale, FALSE, FALSE, 0);  
  
    // controls_hbox  
    controls_hbox = gtk_hbox_new(TRUE, 6);  
    gtk_box_pack_start_defaults(GTK_BOX(main_vbox), controls_hbox);  
  
    // 播放按钮  
    play_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);  
    // 设置“敏感”属性，FALSE 表示为灰色，不响应鼠标键盘事件  
    gtk_widget_set_sensitive(play_button, FALSE);  
    g_signal_connect(G_OBJECT(play_button), "clicked", G_CALLBACK(play_clicked), NULL);  
    gtk_box_pack_start_defaults(GTK_BOX(controls_hbox), play_button);  
      
    // 暂停按钮,为使按下时停留在按下状态，使用GtkToggleButton  
    pause_button = gtk_toggle_button_new_with_label(GTK_STOCK_MEDIA_PAUSE);  
    // 将按钮设置为固化按钮  
    gtk_button_set_use_stock(GTK_BUTTON(pause_button), TRUE);  
    gtk_widget_set_sensitive(pause_button, FALSE);  
    g_signal_connect(G_OBJECT(pause_button), "clicked", G_CALLBACK(pause_clicked), NULL);  
    gtk_box_pack_start_defaults(GTK_BOX(controls_hbox), pause_button);  
  
    // 停止按钮  
    stop_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_STOP);  
    gtk_widget_set_sensitive(stop_button, FALSE);  
    g_signal_connect(G_OBJECT(stop_button), "clicked", G_CALLBACK(stop_clicked), NULL);  
    gtk_box_pack_start_defaults(GTK_BOX(controls_hbox), stop_button);       
      
    // status_hbox  
    status_hbox = gtk_hbox_new(TRUE, 0);  
    gtk_box_pack_start(GTK_BOX(main_vbox), status_hbox, FALSE, FALSE, 0);  
    // 状态标签  
    status_label = gtk_label_new("<b>已停止</b>");  
    gtk_label_set_use_markup(GTK_LABEL(status_label), TRUE);  
    gtk_misc_set_alignment(GTK_MISC(status_label), 0.0, 0.5);  
    gtk_box_pack_start(GTK_BOX(status_hbox), status_label, TRUE, TRUE, 0);  
    // 时间标签     
    time_label = gtk_label_new("00:00:00");  
    gtk_misc_set_alignment(GTK_MISC(time_label), 0.5, 1.0);  
    gtk_box_pack_start(GTK_BOX(status_hbox), time_label, TRUE, TRUE, 0);  
     
    return main_vbox;  
}  
  
  
// destory main window  
static void destroy(GtkWidget *widget, gpointer data)  
{  
    gtk_main_quit();  
}  
// 更新播放时间  
void gui_update_time(const gchar *time, const gint64 position, const gint64 length)  
{  
	g_print("gui_update_time\n");  
}  
// 更新播放状态  
void gui_status_update(PlayerState state)  
{  
    g_print("gui_status_update\n");  
}  
/*  
static gboolean bus_callback(GstBus *bus, GstMessage *message, gpointer data)  
{  
   g_print("bus_callback\n");  
}  
*/
 /* 
static gboolean build_gstreamer_pipeline(const gchar *uri)  
{  
   g_print("build_gstreamer_pipeline\n");  
   return TRUE;
}  
*/

// load file to play  
gboolean load_file(const gchar *uri)  
{  
	g_print("load_file\n"); 
	if(playeropen(uri))
		return TRUE;
	return FALSE;
}  

/*
static gboolean update_time_callback(GstElement *pipeline)  
{  
	g_print("update_time_callback\n");  
}  
 */
gboolean play_file()  
{  
	g_print("play_file\n");  
	return TRUE;
}  
  
/* Attempt to seek to the given percentage through the file */  
void seek_to(gdouble percentage)  
{  
	g_print("seek_to\n");  
}  
  
int main(int argc, char **argv[])  
{  

	int i=0;
	//printf("You have inputed total %d argments\n",argc);  
	for(i=0;i<argc;i++)  
	{  
		//printf("arg%d : %s\n",i,argv[i]);  
	} 
 
    // 初始化 GTK+  
    gtk_init(&argc, &argv);  
    //gst_init(&argc, &argv);  
    // 创建窗口  
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    // 设置窗口标题  
    gtk_window_set_title(GTK_WINDOW(main_window), "FFMPEG Player");  
    // 主窗口销毁句柄  
    g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(destroy), NULL);  
    // 创建主窗口GUI  
    gtk_container_add(GTK_CONTAINER(main_window), build_gui());  
    // 显示  
    gtk_widget_show_all(GTK_WIDGET(main_window));     
    // 开始主循环  
    gtk_main();  
  
    return 0;  
}  


