#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
 #include <sys/time.h>

#include "mp4v2_mp4.h"

/* AAC object types */
enum
{
    GF_M4A_AAC_MAIN = 1,
    GF_M4A_AAC_LC = 2,
    GF_M4A_AAC_SSR = 3,
    GF_M4A_AAC_LTP = 4,
    GF_M4A_AAC_SBR = 5,
    GF_M4A_AAC_SCALABLE = 6,
    GF_M4A_TWINVQ = 7,
    GF_M4A_CELP = 8,
    GF_M4A_HVXC = 9,
    GF_M4A_TTSI = 12,
    GF_M4A_MAIN_SYNTHETIC = 13,
    GF_M4A_WAVETABLE_SYNTHESIS = 14,
    GF_M4A_GENERAL_MIDI = 15,
    GF_M4A_ALGO_SYNTH_AUDIO_FX = 16,
    GF_M4A_ER_AAC_LC = 17,
    GF_M4A_ER_AAC_LTP = 19,
    GF_M4A_ER_AAC_SCALABLE = 20,
    GF_M4A_ER_TWINVQ = 21,
    GF_M4A_ER_BSAC = 22,
    GF_M4A_ER_AAC_LD = 23,
    GF_M4A_ER_CELP = 24,
    GF_M4A_ER_HVXC = 25,
    GF_M4A_ER_HILN = 26,
    GF_M4A_ER_PARAMETRIC = 27,
    GF_M4A_SSC = 28,
    GF_M4A_AAC_PS = 29,
    GF_M4A_LAYER1 = 32,
    GF_M4A_LAYER2 = 33,
    GF_M4A_LAYER3 = 34,
    GF_M4A_DST = 35,
    GF_M4A_ALS = 36
};

uint64_t os_get_reltime_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}
 
 
int ReadOneNaluFromBuf(const unsigned char *buffer,unsigned int nBufferSize,unsigned int offSet,MP4ENC_NaluUnit &nalu)
{
    unsigned int i = offSet;
    while(i<nBufferSize)
    {
        if(buffer[i++] == 0x00 &&
            buffer[i++] == 0x00 &&
            buffer[i++] == 0x00 &&
            buffer[i++] == 0x01
            )
        {
            unsigned int pos = i;
            while (pos<nBufferSize)
            {
                if(buffer[pos++] == 0x00 &&
                    buffer[pos++] == 0x00 &&
                    buffer[pos++] == 0x00 &&
                    buffer[pos++] == 0x01
                    )
                {
                    break;
                }
            }
            if(pos == nBufferSize)
            {
                nalu.size = pos-i;  
            }
            else
            {
                nalu.size = (pos-4)-i;
            }
 
            nalu.type = buffer[i]&0x1f;
            nalu.data =(unsigned char*)&buffer[i];
            // printf("nalu.type: %02d, nalu.data: %p, nalu.size: %d\n", nalu.type, nalu.data, nalu.size);
            return (nalu.size+i-offSet);
        }
    }
    return 0;
}
 
/* Returns the sample rate index */
static int GetSRIndex(unsigned int sampleRate)
{
    if (92017 <= sampleRate) return 0;
    if (75132 <= sampleRate) return 1;
    if (55426 <= sampleRate) return 2;
    if (46009 <= sampleRate) return 3;
    if (37566 <= sampleRate) return 4;
    if (27713 <= sampleRate) return 5;
    if (23004 <= sampleRate) return 6;
    if (18783 <= sampleRate) return 7;
    if (13856 <= sampleRate) return 8;
    if (11502 <= sampleRate) return 9;
    if (9391 <= sampleRate) return 10;
    return 11;
}
 
static void GetAudioSpecificConfig(uint8_t AudioType, uint8_t SampleRateID, uint8_t Channel, uint8_t *pHigh, uint8_t *pLow)
{
    uint16_t Config;
 
    Config = 0xffff&(AudioType & 0x1f);
    Config <<= 4;
    Config |= SampleRateID & 0x0f;
    Config <<= 4;
    Config |= Channel & 0x0f;
    Config <<= 3;
 
    *pLow  = Config & 0xff;
    Config >>= 8;
    *pHigh = Config & 0xff;
}
 
int Mp4v2WriteH264toMP4(MP4V2_MP4_T *pHandle, unsigned char *buffer, unsigned int frame_size)
{
    char nalu_type = buffer[4] & 0x1f;
    unsigned char *nalu_data = (unsigned char *) &buffer[4];
    unsigned int nalu_size = frame_size - 4;
    uint64_t nowvoltime = os_get_reltime_ms();
    printf("nalu_type: %02d, frame_size: %d\n", nalu_type, frame_size);
    if(nalu_type == 0x07 && 1 == pHandle->isFirstSPS) // sps    
    {    
        MP4SetTimeScale(pHandle->hMp4File,pHandle->m_nTimeScale);
        printf("isFirstSPS.\n");    
        pHandle->m_videoId = MP4AddH264VideoTrack    
                            (   pHandle->hMp4File,     
                                    pHandle->m_nTimeScale,     
                                    pHandle->m_nTimeScale / pHandle->m_nFrameRate,     
                                    pHandle->m_nWidth,//1080,  
                                    pHandle->m_nHeight,//720,  
                                    nalu_data[1],                 // sps[1] AVCProfileIndication    
                                    nalu_data[2],                 // sps[2] profile_compat    
                                    nalu_data[3],                 // sps[3] AVCLevelIndication    
                                    3);                             // 4 bytes length before each NAL unit    
        if (pHandle->m_videoId == MP4_INVALID_TRACK_ID)    
        {    
            //MP4Close(pHandle->hMp4File,0); //add in 20180619
            printf("add video track failed.\n");    
            return 1;
        } 
        // MP4SetVideoProfileLevel(pHandle->hMp4File, 0x7F);
        MP4SetVideoProfileLevel(pHandle->hMp4File, 0x01);
        MP4AddH264SequenceParameterSet(pHandle->hMp4File, pHandle->m_videoId, nalu_data, nalu_size);                                             
        pHandle->isFirstSPS = 0;  
    } 
    else if(nalu_type == 0x08 && 1 == pHandle->isFirstPPS) // pps   
    {    
        MP4AddH264PictureParameterSet(pHandle->hMp4File, pHandle->m_videoId, nalu_data, nalu_size); 
        pHandle->isFirstPPS = 0;   
        printf("isFirstPPS.\n");    
    }  
    else if(nalu_type == 0x06)  //sei
    {
 
    }
    else if(!pHandle->isFirstSPS && !pHandle->isFirstPPS)
    {   
        bool success;
        char hander[4]={0};
        memcpy(hander, buffer, 4);
        buffer[0] = nalu_size >> 24;    
        buffer[1] = nalu_size >> 16;    
        buffer[2] = nalu_size >> 8;    
        buffer[3] = nalu_size & 0xff;                 
        if(1 == pHandle->isFirstFrame)   
        {
            if(nalu_type == 0x05)   //第一帧是IDR
            {
                printf("isFirstFrame.\n"); 
                success = MP4WriteSample(pHandle->hMp4File, pHandle->m_videoId, buffer, frame_size, pHandle->m_nTimeScale / pHandle->m_nFrameRate, 0, 1);
                if (!success) {
                    printf("MP4WriteSample: fail.\n"); 
                }
                // else {
                //     printf("MP4WriteSample: success.\n"); 
                // }
                                
                pHandle->videotime=nowvoltime;
                pHandle->audiotime = nowvoltime;
                pHandle->isFirstFrame = 0;
            }
        }   
        else
        {
            char isSyncSample = 0; 
            if(nalu_type == 0x05) {
                isSyncSample = 1;
            }
            pthread_mutex_lock(&pHandle->mutex);
            success = MP4WriteSample(pHandle->hMp4File, pHandle->m_videoId, buffer, frame_size, (nowvoltime-pHandle->videotime) * 90, 0, isSyncSample);
            if (!success) {
                printf("MP4WriteSample: fail.\n"); 
            }
            // else {
            //     printf("MP4WriteSample: success.\n"); 
            // }
            pthread_mutex_unlock(&pHandle->mutex);        
            pHandle->videotime=nowvoltime;
        }  
        memcpy(buffer, hander, 4);
    }         
    return 0; 
}
 
int Mp4v2WriteAACtoMP4(MP4V2_MP4_T *pHandle, unsigned char *buffer, unsigned int frame_size)
{
    if(!pHandle->isFirstFrame)
    {
        //去除adts头
        // const unsigned char *buff = &buffer[7];
        // int size = frame_size -7;
        bool success;
        uint64_t nowvoltime = os_get_reltime_ms();
        uint64_t duration = (nowvoltime - pHandle->audiotime) * (pHandle->samplerate / 1000);
        pHandle->audiotime = nowvoltime;
        pthread_mutex_lock(&pHandle->mutex);
        success = MP4WriteSample(pHandle->hMp4File, pHandle->m_audioId, buffer, frame_size, duration, 0, 1);
        pthread_mutex_unlock(&pHandle->mutex);
        if(!success) { 
            printf("MP4WriteSample failed 3600\n");
            return 0;    
        }
    }
    return 0;
}

MP4V2_MP4_T *Mp4v2CreateMP4File(const char *pFileName,int width,int height,int timeScale/* = 90000*/,int frameRate/* = 25*/)
{
    MP4V2_MP4_T *pHandle = (MP4V2_MP4_T *)malloc(sizeof(MP4V2_MP4_T));
    if(pFileName == NULL || pHandle ==NULL)
    {
        return NULL;
    }
    pthread_mutex_init(&pHandle->mutex, NULL);
    
    // create mp4 file
    //MP4_CREATE_64BIT_DATA 标记允许文件总大小超过64位的数据。我理解的是，允许单个mp4文件的容量超过2^32KB=4GB。
    pHandle->hMp4File = MP4Create(pFileName);
    // pHandle->hMp4File = MP4CreateEx(pFileName,  0, 1, 1, 0, 0, 0, 0);//创建mp4文件
    if (pHandle->hMp4File == MP4_INVALID_FILE_HANDLE)
    {
        printf("ERROR:Open file fialed.\n");
        return NULL;
    }
 
    //添加aac音频
    // pHandle->samplerate = 32000;
    // pHandle->samplerate = 8000;
    pHandle->samplerate = 44100;
    // MP4Duration duration = 1024;
    pHandle->m_audioId = MP4AddAudioTrack(pHandle->hMp4File, pHandle->samplerate, MP4_INVALID_DURATION, MP4_MPEG4_AUDIO_TYPE);
    if (pHandle->m_audioId == MP4_INVALID_TRACK_ID)
    {
        printf("add audio track failed.\n");
    }
    MP4SetAudioProfileLevel(pHandle->hMp4File, 0x2);
    if(1)
    {
        bool success;
        uint8_t Type = GF_M4A_AAC_LC;
        uint8_t SampleRate = GetSRIndex(pHandle->samplerate);
        uint8_t Channel = 1;
        uint8_t  aacInfo[2];
        unsigned long  aacInfoSize = 2; 
        GetAudioSpecificConfig(Type,SampleRate, Channel, &aacInfo[0], &aacInfo[1]);
        printf("aacInfo=%#x, %#x\n", aacInfo[0], aacInfo[1]);
        success = MP4SetTrackESConfiguration(pHandle->hMp4File, pHandle->m_audioId, (uint8_t *)&aacInfo, aacInfoSize );
        if(!success) {
            printf("MP4SetTrackESConfiguration failed\n");
        }
    }
 
    pHandle->m_nWidth = width;
    pHandle->m_nHeight = height;
    pHandle->m_nTimeScale = timeScale;
    pHandle->m_nFrameRate = frameRate;
    pHandle->isFirstPPS = 1;
    pHandle->isFirstSPS = 1;
    pHandle->isFirstFrame = 1;
    
    return pHandle;
}
 
void Mp4v2CloseMP4File(MP4V2_MP4_T *pHandle)
{
    if(pHandle)
    {
        if(pHandle->hMp4File)
        {
            MP4Close(pHandle->hMp4File, MP4_CLOSE_DO_NOT_COMPUTE_BITRATE);
            // MP4Close(pHandle->hMp4File, 0);
            pHandle->hMp4File = NULL;
        }
 
        pthread_mutex_destroy(&pHandle->mutex);
        free(pHandle);
        pHandle = NULL;
    }
}