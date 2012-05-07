#include "videothread.h"

VideoThread::VideoThread(QList<AVPacket> &q, QMutex *m, AVCodecContext *_pCodecCtx, AVFormatContext *_pFormatCtx, int vs, QObject *parent) :
    QThread(parent), queue(q), mutex(m), pCodecCtx(_pCodecCtx), pFormatCtx(_pFormatCtx), videoStream(vs)
{
    pFrame=0;
    pFrameRGB=0;
    buffer=0;
    img_convert_ctx=0;
    timeoffset=-1;

    LastLastFrameTime=INT_MIN;       // Last last must be small to handle the seek well
    LastFrameTime=0;
    LastLastFrameNumber=INT_MIN;
    LastFrameNumber=0;
    DesiredFrameTime=DesiredFrameNumber=0;
    LastFrameOk=false;

    // Allocate video frame
    pFrame=avcodec_alloc_frame();

    // Allocate an AVFrame structure
    pFrameRGB=avcodec_alloc_frame();

    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,pCodecCtx->height);
    buffer=new uint8_t[numBytes];

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
        pCodecCtx->width, pCodecCtx->height);

    connect(&playTimer, SIGNAL(timeout()), this, SLOT(OnPlayTimeout()));

    int den = pCodecCtx->time_base.den;
    int num = pCodecCtx->time_base.num;
    int interval = 2* (double)num*1000 / (double)den;
    cout << "den: " << den << "num: " << num << "interval: " << interval << endl;
    playTimer.start(interval);
}

VideoThread::~VideoThread()
{
    // Free the RGB image
    if(buffer)
       delete [] buffer;

    // Free the YUV frame
    if(pFrame)
       av_free(pFrame);

    // Free the RGB frame
    if(pFrameRGB)
       av_free(pFrameRGB);
}

bool VideoThread::seekNextFrame()
{
   bool ret = decodeSeekFrame(DesiredFrameNumber+1);

   if(ret)
      DesiredFrameNumber++;   // Only updates the DesiredFrameNumber if we were successful in getting that frame
   else
      LastFrameOk=false;      // We didn't find the next frame (e.g. seek out of range) - mark we don't know where we are.
   return ret;
}

bool VideoThread::getFrame(QImage&img,qint64 *effectiveframenumber,qint64 *effectiveframetime,qint64 *desiredframenumber,qint64 *desiredframetime)
{
   img = LastFrame;

   if(effectiveframenumber)
      *effectiveframenumber = LastFrameNumber;
   if(effectiveframetime)
      *effectiveframetime = LastFrameTime;
   if(desiredframenumber)
      *desiredframenumber = DesiredFrameNumber;
   if(desiredframetime)
      *desiredframetime = DesiredFrameTime;

   //printf("getFrame. Returning valid? %s. Desired %d @ %d. Effective %d @ %d\n",LastFrameOk?"yes":"no",DesiredFrameNumber,DesiredFrameTime,LastFrameNumber,LastFrameTime);

   return LastFrameOk;
}

bool VideoThread::decodeSeekFrame(qint64 after)
{

   //printf("decodeSeekFrame. after: %d. LLT: %d. LT: %d. LLF: %d. LF: %d. LastFrameOk: %d.\n",after,LastLastFrameTime,LastFrameTime,LastLastFrameNumber,LastFrameNumber,(int)LastFrameOk);

   // If the last decoded frame satisfies the time condition we return it
   //if( after!=-1 && ( LastDataInvalid==false && after>=LastLastFrameTime && after <= LastFrameTime))
    /*
   if( 0 && after!=-1 && ( LastFrameOk==true && after>=LastLastFrameNumber && after <= LastFrameNumber))
   {
      // This is the frame we want to return

      // Compute desired frame time
      ffmpeg::AVRational millisecondbase = {1, 1000};
      DesiredFrameTime = ffmpeg::av_rescale_q(after,pFormatCtx->streams[videoStream]->time_base,millisecondbase);

      printf("Returning already available frame %d @ %d. DesiredFrameTime: %d\n",LastFrameNumber,LastFrameTime,DesiredFrameTime);

      return true;
   }*/

         // Is this a packet from the video stream -> decode video frame

         int frameFinished;
         mutex->lock();
         if (queue.empty()) {mutex->unlock(); return false;}
         AVPacket packet = queue.takeFirst();
         mutex->unlock();
         avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished,&packet);

         //printf("used %d out of %d bytes\n",len,packet.size);

         //printf("Frame type: ");
         //if(pFrame->pict_type == FF_B_TYPE)
         //   printf("B\n");
         //else if (pFrame->pict_type == FF_I_TYPE)
         //   printf("I\n");
         //else
         //   printf("P\n");


         //printf("codecctx time base: num: %d den: %d\n",pCodecCtx->time_base.num,pCodecCtx->time_base.den);
         //printf("formatctx time base: num: %d den: %d\n",pFormatCtx->streams[videoStream]->time_base.num,pFormatCtx->streams[videoStream]->time_base.den);
         //printf("pts: %ld\n",pts);
         //printf("dts: %ld\n",dts);




         // Did we get a video frame?
         if(frameFinished)
         {
            AVRational millisecondbase = {1, 1000};
            qint64 f = packet.dts;
            qint64 t = av_rescale_q(packet.dts,pFormatCtx->streams[videoStream]->time_base,millisecondbase);
            if(LastFrameOk==false)
            {
               LastFrameOk=true;
               LastLastFrameTime=LastFrameTime=t;
               LastLastFrameNumber=LastFrameNumber=f;
            }
            else
            {
               // If we decoded 2 frames in a row, the last times are okay
               LastLastFrameTime = LastFrameTime;
               LastLastFrameNumber = LastFrameNumber;
               LastFrameTime=t;
               LastFrameNumber=f;
            }
            if (timeoffset < 0) timeoffset = LastFrameTime;
            //printf("Frame %d @ %d. LastLastT: %d. LastLastF: %d. LastFrameOk: %d\n",LastFrameNumber,LastFrameTime,LastLastFrameTime,LastLastFrameNumber,(int)LastFrameOk);

            // Is this frame the desired frame?
            if(after==-1 || LastFrameNumber>=after)
            {
               // It's the desired frame

               // Convert the image format (init the context the first time)
               int w = pCodecCtx->width;
               int h = pCodecCtx->height;

               img_convert_ctx = sws_getCachedContext(img_convert_ctx,w, h, pCodecCtx->pix_fmt, w, h, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

               if(img_convert_ctx == NULL)
               {
                  cout << "Cannot initialize the conversion context!" << endl;
                  return false;
               }
               sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

               // Convert the frame to QImage
               LastFrame=QImage(w,h,QImage::Format_RGB888);

               for(int y=0;y<h;y++)
                  memcpy(LastFrame.scanLine(y),pFrameRGB->data[0]+y*pFrameRGB->linesize[0],w*3);

               // Set the time
               DesiredFrameTime = av_rescale_q(after,pFormatCtx->streams[videoStream]->time_base,millisecondbase);
               LastFrameOk=true;

            } // frame of interest
         }  // frameFinished
        // stream_index==videoStream
      av_free_packet(&packet);      // Free the packet that was allocated by av_read_frame

   //printf("Returning new frame %d @ %d. LastLastT: %d. LastLastF: %d. LastFrameOk: %d\n",LastFrameNumber,LastFrameTime,LastLastFrameTime,LastLastFrameNumber,(int)LastFrameOk);
   //printf("\n");
   return true;   // done indicates whether or not we found a frame
}

int VideoThread::getVideoLengthMs()
{
   int secs = pFormatCtx->duration / AV_TIME_BASE;
   int us = pFormatCtx->duration % AV_TIME_BASE;
   int l = secs*1000 + us/1000;

   return l;
}

int VideoThread::getCurrentMs()
{
    if (timeoffset < 0) return 0;
    return LastLastFrameTime - timeoffset;
}

void VideoThread::run()
{

}

void VideoThread::OnPlayTimeout()
{
    seekNextFrame();
    emit display();
}
