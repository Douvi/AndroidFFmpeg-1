#include "trimVideoFFmpeg.h"
#include <assert.h>


#define TAG "TrimVideoFFmpeg-NATIVE"
#define ERROR -1
#define SUCCESS 0

int startTime;
int endTime;
int videoStream;
int inVideoStreamIndex = 0;
int outputed;
int frameFinished;

AVCodecContext *pOutCodecCtxOrig = NULL;
AVCodecContext *pOutCodecCtx = NULL;
AVCodec *pOutCodec = NULL;
AVFormatContext *pOutFormatCtx = NULL;

AVCodecContext *pInCodecCtxOrig = NULL;
AVCodecContext *pInCodecCtx = NULL;
AVCodec *pInCodec = NULL;
AVFormatContext *pInFormatCtx = NULL;

AVStream *pInVideoStream = NULL;

/*
 * Class:     com_example_titants_videoselection_util_TrimVideoFFmpeg
 * Method:    naTrimVideo
 * Signature: (Ljava/lang/String;Ljava/lang/String;II)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_titants_videoselection_util_TrimVideoFFmpeg_naTrimVideo
  (JNIEnv *pEnv, jobject jObj, jstring sSrcPath, jstring sDstPath, jstring sFileName, jint iStartTime, jint iEndTime)
{
    const char *inSrcPath = (*pEnv)->GetStringUTFChars(pEnv, sSrcPath, 0);
    const char *outSrcPath = (*pEnv)->GetStringUTFChars(pEnv, sDstPath, 0);
    const char *cfileName = (*pEnv)->GetStringUTFChars(pEnv, sFileName, 0);

    startTime = (int) iStartTime;
    endTime = (int) iEndTime;
    int res = 0;

    __android_log_print(ANDROID_LOG_INFO, TAG, "---> START METHOD naTrimVideo <----");
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> sFileName : %s", cfileName);


    av_register_all();
//    avcodec_register_all();

    if (openFile(&pInFormatCtx, inSrcPath) != SUCCESS)
    {
        return ERROR;
    }

    if (getCodecVideo(pInFormatCtx, &pInCodecCtx, &pInCodecCtxOrig, &pInCodec) != SUCCESS)
    {
        return ERROR;
    }

    /* allocate the output media context */
    avformat_alloc_output_context2(&pOutFormatCtx, NULL, NULL, outSrcPath);
    if (!pOutFormatCtx) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Could not deduce output format from file extension: using MPEG.");
        avformat_alloc_output_context2(&pOutFormatCtx, NULL, "mpeg", outSrcPath);
    }
    if (!pOutFormatCtx) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "ERROR CAN NOT FOUND pOutFormatCtx!!!!!");
        return ERROR;
    }
    AVOutputFormat *fmt = pOutFormatCtx->oformat;


//    avformat_alloc_output_context2(&pOutFormatCtx, NULL, NULL, outSrcPath);
//    pOutCodec = avcodec_find_encoder_by_name("mp4");
//      pOutCodec = avcodec_find_encoder_by_name(cfileName);

//    AVOutputFormat *outfmt = av_guess_format(cfileName, NULL, NULL);
//    pOutFormatCtx = avformat_alloc_context();
//    assert(pOutFormatCtx);
//    pOutFormatCtx->oformat  = av_guess_format(NULL, outSrcPath, NULL);
//    assert(pOutFormatCtx->oformat);
//
    pOutCodec = avcodec_find_decoder(pOutFormatCtx->oformat->video_codec);
    pOutCodecCtx = avcodec_alloc_context3(pOutCodec);

    pOutCodecCtxOrig = avcodec_alloc_context3(pOutCodec);
    if(avcodec_copy_context(pOutCodecCtxOrig, pOutCodecCtx) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't copy codec context");
//        __android_log_write(ANDROID_LOG_ERROR, TAG, stderr);
        return ERROR; // Error copying codec context
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> COPY CODEC CONTEXT");

    // Open codec
    if(avcodec_open2(pOutCodecCtx, pOutCodec, NULL)<0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't copy codec context");
//        __android_log_write(ANDROID_LOG_ERROR, TAG, stderr);
        return ERROR; // Could not open codec
    }

    av_dump_format(pOutFormatCtx, 0, outSrcPath, 1);

    __android_log_print(ANDROID_LOG_INFO, TAG, "---> AVCodec : %p", pOutCodec);
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> AVCodecContext : %p", pOutCodecCtx);
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> OutFormatCtx : %p", pOutFormatCtx);
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> pOutFormatCtx->oformat : %p", pOutFormatCtx->oformat);
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> OutFormatCtx->video_codec_id : %d", pOutFormatCtx->oformat->video_codec);
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> NB STREAM : %i", pOutFormatCtx->nb_streams);

//    __android_log_print(ANDROID_LOG_ERROR, TAG, "---> pOutCodec : %p", pOutCodec);

//    if (openFile(&pOutFormatCtx, inSrcPath) == -1)
//    {
//        return -1;
//    }
//
//    if (getCodecVideo(pOutFormatCtx, &pOutCodecCtx, &pOutCodecCtxOrig, &pOutCodec) == SUCCESS)
//    {
//        return ERROR;
//    }

    if (convert_and_cut(outSrcPath, (float)startTime, (float)endTime) == -1) {
        return -1;
    }

    return SUCCESS;
}

static int openFile(AVFormatContext **pFormatContext, const char *cPath) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> OPEN FILE <---");
    __android_log_print(ANDROID_LOG_INFO, TAG, "         ---> FILE PATH : %s", cPath);

    //    Open video file
    if(avformat_open_input(pFormatContext, cPath, NULL, NULL) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Tag", "         ----> Error avformat_open_input");
        return ERROR; // Couldn't open file
    }

    // Retrieve stream information
    if(avformat_find_stream_info(*pFormatContext, NULL) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Tag", "         ----> Error avformat_find_stream_info");
        return ERROR;  // Couldn't find stream information
    }

    // Dump information about file onto standard error
//    av_dump_format(pFormatContext, 0, cPath, 0);

    __android_log_print(ANDROID_LOG_INFO, TAG, "        ---> FORMAT CONTEXT : %p", pFormatContext);
    __android_log_print(ANDROID_LOG_INFO, TAG, "        ---> OPEN FILE WITH SUCCESS ");
    return SUCCESS;
}

static int getCodecVideo(AVFormatContext *pFormatCtx, AVCodecContext **pCodecCtx, AVCodecContext **pCodecCtxOrig, AVCodec **pCodec) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> OPEN CODEC <---");
    int i;

    // Find the first video stream
    videoStream = -1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    if(videoStream == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Didn't find a video stream");
//        __android_log_write(ANDROID_LOG_ERROR, TAG, stderr);
        return ERROR; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    *pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> FOUND CODEC CONTEXT");

    // Find the decoder for the video stream
    *pCodec = avcodec_find_decoder((*pCodecCtx)->codec_id);
    if(pCodec == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unsupported codec!");
//        __android_log_write(ANDROID_LOG_ERROR, TAG, stderr);
        return ERROR; // Codec not found
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> FOUND CODEC");

    // Copy context
    *pCodecCtxOrig = avcodec_alloc_context3(*pCodec);
    if(avcodec_copy_context(*pCodecCtxOrig, *pCodecCtx) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't copy codec context");
//        __android_log_write(ANDROID_LOG_ERROR, TAG, stderr);
        return ERROR; // Error copying codec context
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> COPY CODEC CONTEXT");

    // Open codec
    if(avcodec_open2(*pCodecCtx, *pCodec, NULL)<0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't copy codec context");
//        __android_log_write(ANDROID_LOG_ERROR, TAG, stderr);
        return ERROR; // Could not open codec
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> OPEN CODEC ORIG : %p", pCodecCtxOrig);
    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> OPEN CODEC : %p", pCodecCtx);
    __android_log_print(ANDROID_LOG_INFO, TAG, "            ---> OPEN CODEC WITH SUCCESS ");

    return SUCCESS;
}

static int convert_and_cut(const char *file, float starttime, float endtime) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> START CUTTING VIDEO <---");

    AVFrame *frame;
    AVPacket inPacket, outPacket;

    if(avio_open(&pOutFormatCtx->pb, file, AVIO_FLAG_WRITE) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "--->  convert(): cannot open out file - avio_open()");
        return ERROR;
    }

    // seek to the start time you wish.
    // BEGIN
    AVRational default_timebase;
    default_timebase.num = 1;
    default_timebase.den = AV_TIME_BASE;

    pInVideoStream = pInFormatCtx->streams[inVideoStreamIndex];

    // suppose you have access to the "pInVideoStream" of course
    int64_t starttime_int64 = av_rescale_q((int64_t)( starttime * AV_TIME_BASE ), default_timebase, pInVideoStream->time_base);
    int64_t endtime_int64 = av_rescale_q((int64_t)( endtime * AV_TIME_BASE ), default_timebase, pInVideoStream->time_base);

    __android_log_print(ANDROID_LOG_INFO, TAG, "---> START TIME : %ld", (long)starttime_int64);
    __android_log_print(ANDROID_LOG_INFO, TAG, "---> END TIME : %ld", (long)endtime_int64);

    if(avformat_seek_file(pInFormatCtx, inVideoStreamIndex, INT64_MIN, starttime_int64, INT64_MAX, 0) < 0) {
        // error... do something...
        __android_log_print(ANDROID_LOG_ERROR, TAG, "---> avformat_seek_file()");
        return ERROR;
    }

    avcodec_flush_buffers(pInVideoStream->codec);
    // END

    int ret = avformat_write_header(pOutFormatCtx, NULL);
    if(ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "NDK: [%s(%d) - %s]", "Could not write header", ret, av_err2str(ret));
        return ERROR;
    }

    frame = avcodec_alloc_frame();
    av_init_packet(&inPacket);

    // you used avformat_seek_file() to seek CLOSE to the point you want... in order to give precision to your seek,
    // just go on reading the packets and checking the packets PTS (presentation timestamp)
    while(av_read_frame(pInFormatCtx, &inPacket) >= 0) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "---> into av_read_frame");
        if(inPacket.stream_index == inVideoStreamIndex) {
            __android_log_print(ANDROID_LOG_INFO, TAG, "---> into av_read_frame -> if ");
            avcodec_decode_video2(pInCodecCtx, frame, &frameFinished, &inPacket);
            // this line guarantees you are getting what you really want.
            if(frameFinished && frame->pkt_pts >= starttime_int64 && frame->pkt_pts <= endtime_int64) {
                __android_log_print(ANDROID_LOG_INFO, TAG, "---> into av_read_frame -> if -> if ");
                av_init_packet(&outPacket);
                avcodec_encode_video2(pOutCodecCtx, &outPacket, frame, &outputed);
                if(outputed) {
                    __android_log_print(ANDROID_LOG_INFO, TAG, "---> into av_read_frame -> if -> if -> if ");
                    if (av_write_frame(pOutFormatCtx, &outPacket) != 0) {
                        __android_log_print(ANDROID_LOG_INFO, TAG, "---> into av_read_frame -> if -> if -> if");
                        __android_log_write(ANDROID_LOG_ERROR, "Tag", "---> convert(): error while writing video frame - av_write_frame");
//                        fprintf(stderr, "convert(): error while writing video frame\n");
                        return -1;
                    }
                }
                av_free_packet(&outPacket);
            }

            // exit the loop if you got the frames you want.
            if(frame->pkt_pts > endtime_int64) {
                break;
            }
        }
    }

    av_write_trailer(pOutFormatCtx);
    av_free_packet(&inPacket);

    __android_log_print(ANDROID_LOG_INFO, TAG, "----> VIDEO TRIMMED <----");
    return SUCCESS;
}