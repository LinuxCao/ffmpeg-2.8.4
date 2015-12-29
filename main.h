


/* 
 * main.h 
 */  
  
#ifndef MAIN_H  
#define MAIN_H  
  
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

// 常量定义
#define DEF_PKT_QUEUE_ASIZE   50
#define DEF_PKT_QUEUE_VSIZE   50
  
 typedef enum {  
    STATE_STOP,  
    STATE_PLAY,  
    STATE_PAUSE  
} PlayerState; 

typedef struct {
    long       fsize;
    long       asize;
    long       vsize;
    long       fhead;
    long       ftail;
    long       ahead;
    long       atail;
    long       vhead;
    long       vtail;
    long       apktnum;
    long       vpktnum;
    sem_t      fsemr;
    sem_t      asemr;
    sem_t      asemw;
    sem_t      vsemr;
    sem_t      vsemw;
    AVPacket  *bpkts; // packet buffers
    AVPacket **fpkts; // free packets
    AVPacket **apkts; // audio packets
    AVPacket **vpkts; // video packets
} PKTQUEUE;

// 内部类型定义
typedef struct
{
    // audio
    AVFormatContext *pAVFormatContext;
    AVCodecContext  *pAudioCodecContext;
    int              iAudioStreamIndex;
    double           dAudioTimeBase;

    // video
    AVCodecContext  *pVideoCodecContext;
    int              iVideoStreamIndex;
    double           dVideoTimeBase;

    // render
    int              nRenderMode;
    void            *hCoreRender;

    // thread
    #define PS_D_PAUSE  (1 << 0)  // demux pause
    #define PS_A_PAUSE  (1 << 1)  // audio decoding pause
    #define PS_V_PAUSE  (1 << 2)  // video decoding pause
    #define PS_R_PAUSE  (1 << 3)  // rendering pause
    #define PS_CLOSE    (1 << 4)  // close player
    int              nPlayerStatus;
    pthread_t        hAVDemuxThread;
    pthread_t        hADecodeThread;
    pthread_t        hVDecodeThread;

    // packet queue
    PKTQUEUE         PacketQueue;
} PLAYER;
 
void gui_status_update(PlayerState state);  
void gui_update_time(const gchar *time, const gint64 position, const gint64 length);  
   
gboolean load_file(const gchar *uri);  
void seek_to(gdouble percentage);  
gboolean play_file();
  
#endif 
