#ifndef _MP4V2_MP4_H_
#define _MP4V2_MP4_H_
 
#include "mp4v2/mp4v2.h"
#include <pthread.h>
 
// NALU单元
typedef struct _MP4ENC_NaluUnit
{
    int type;
    int size;
    unsigned char *data;
}MP4ENC_NaluUnit;
 
typedef struct mp4v2_mp4
{
    int m_nWidth;
    int m_nHeight;
    int m_nFrameRate;
    int m_nTimeScale;
    char isFirstPPS;
    char isFirstSPS;
    char isFirstFrame;
    int samplerate;
    uint64_t videotime;
    uint64_t audiotime;
    MP4TrackId m_videoId;
    MP4TrackId m_audioId;
    MP4FileHandle hMp4File;
    pthread_mutex_t mutex;
}MP4V2_MP4_T;
 
// open or creat a mp4 file.
MP4V2_MP4_T *Mp4v2CreateMP4File(const char *fileName,int width,int height,int timeScale,int frameRate);
void Mp4v2CloseMP4File(MP4V2_MP4_T *pHandle);
int Mp4v2WriteH264toMP4(MP4V2_MP4_T *pHandle, unsigned char *buffer, unsigned int frame_size);
int Mp4v2WriteAACtoMP4(MP4V2_MP4_T *pHandle, unsigned char *buffer, unsigned int frame_size);
int ReadOneNaluFromBuf(const unsigned char *buffer,unsigned int nBufferSize,unsigned int offSet,MP4ENC_NaluUnit &nalu);
 
#endif //_MP4V2_MP4_H_