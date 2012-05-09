#include "decodethread.h"



DecodeThread::DecodeThread(QObject *parent) :
    QThread(parent)
{
    InitVars();
    initCodec();
}
/**
   \brief Constructor - opens directly a video
**/

DecodeThread::~DecodeThread()
{
    close();
}

void DecodeThread::InitVars()
{
    ok=false;
    pFormatCtx=0;
    pVideoCodecCtx=0;
    pAudioCodecCtx=0;
    pVideoCodec=0;
    pAudioCodec=0;
    videoMutex = new QMutex;
    audioMutex = new QMutex;
}

void DecodeThread::close()
{
   if(!ok)
      return;

   // Close the codec
   if(pVideoCodecCtx)
      avcodec_close(pVideoCodecCtx);
   if(pAudioCodecCtx)
      avcodec_close(pAudioCodecCtx);

   // Close the video file
   if(pFormatCtx)
      av_close_input_file(pFormatCtx);

   InitVars();
}


bool DecodeThread::initCodec()
{


    //cout << "License: " << avformat_license() << endl;
    //cout << "AVCodec version: " << avformat_version() << endl;
    //cout << "AVFormat configuration: " << avformat_configuration() << endl;

    return true;
}

bool DecodeThread::openFile(QString filename)
{
   // Close last video..
   close();




   // Open video file
   if(avformat_open_input(&pFormatCtx, filename.toStdString().c_str(), NULL, NULL)!=0)
       return false; // Couldn't open file

   // Retrieve stream information
   if(av_find_stream_info(pFormatCtx)<0)
       return false; // Couldn't find stream information


   // Find the first video stream
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);;
   /*
   for(unsigned i=0; i<pFormatCtx->nb_streams; i++)
       if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
       {
           videoStream=i;
           break;
       }
       */
   if(videoStream==-1)
       return false; // Didn't find a video stream
    audioStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, videoStream, NULL, 0);

   // Get a pointer to the codec context for the video stream
   pVideoCodecCtx=pFormatCtx->streams[videoStream]->codec;
   pAudioCodecCtx=pFormatCtx->streams[audioStream]->codec;
   // Find the decoder for the video stream
   pVideoCodec=avcodec_find_decoder(pVideoCodecCtx->codec_id);
   pAudioCodec=avcodec_find_decoder(pAudioCodecCtx->codec_id);
   if(pVideoCodec==NULL && pAudioCodec==NULL)
       return false; // Codec not found

   // Open codec
   if(avcodec_open(pVideoCodecCtx, pVideoCodec)<0 && avcodec_open(pAudioCodecCtx, pAudioCodec)<0)
       return false; // Could not open codec

   // Hack to correct wrong frame rates that seem to be generated by some
   // codecs
   if(pVideoCodecCtx->time_base.num>1000 && pVideoCodecCtx->time_base.den==1)
     pVideoCodecCtx->time_base.den=1000;

   cout << "iformat: " << pFormatCtx->iformat->name << endl;
   cout << "video codec: " << avcodec_get_name(pVideoCodecCtx->codec_id) << endl;
   cout << "audio codec: " << avcodec_get_name(pAudioCodecCtx->codec_id) << endl;


   vthread = new VideoThread(videoQueue, videoMutex, pVideoCodecCtx, pFormatCtx, videoStream, this);

   ok=true;
   return true;
}

bool DecodeThread::isOk()
{
   return ok;
}


void DecodeThread::run()
{
    if (!ok) return;
    int lives = 3;
    forever
    {
        // Read a frame
        //cout << "I'm stucked here!" << endl;
        if(av_read_frame(pFormatCtx, &packet)<0)
        {
            //cout << "Read frame failed!! Remaining Lives: " << lives << endl;
            //if (lives > 0)
            {
                lives--;
                continue;
            }
            //else
            //break;                             // Frame read failed (e.g. end of stream)
        }

        //cout << "Packet of stream " << packet.stream_index << ", size " << packet.size << endl;

        if(packet.stream_index==videoStream)
        {
            videoMutex->lock();
            videoQueue.append(packet);
            videoMutex->unlock();
        }
        else av_free_packet(&packet);
    }
}
