#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mp4v2/mp4v2.h>
#include "maz_com_assert.h"

int dlvl = DINF;

#define FUNC_DUMP   1

#define MAZ_AUDIO_FRAME_SIZE    768

#define MAZ_STREAM_TYPE_AUDIO   0
#define MAZ_STREAM_TYPE_VIDEO   1

typedef struct _HEADER_
{
    int sync;           // 同步码, 值恒为 0x55AA5A5A
    int type;           // 码流类型, 0:音频帧; 1:视频帧
    int num;            // 码流序号
    int len;            // 码流长度
} HEADER;

int maz_split(char *iname);
int maz_h264_get_nalu(FILE *ifile, unsigned char *pnalu);

int main(int argc, char *argv[])
{
    int ret = -1;

    if (argc != 2)
    {
        dlog(DERR, "usage: %s <bin>\n", argv[0]);
        return -1;
    }

    ret = maz_split(argv[1]);
    MAZASSERT_RETVAL(ret, -1, "err: maz_joint");

    return 0;
}

/**
 * @brief 将 AAC 音频和 H.265 视频拼接为一个文件
 * @note 音频采样率为8k, 一帧音频帧包含1024个采样, 1秒8帧音频
 *       视频帧率为 25fps
 *       因此, 1帧音频对应3帧视频, 由于一比三的关系, 只能包含24fps视频,
因此这里最后一帧音频多匹配一帧视频
 */
int maz_split(char *iname)
{
    int i = 0;
    int rcnt = 0;                   // read count
    int wcnt = 0;                   // write count
    int scnt = 0;                   // stream count
    int acnt = 0;
    int vcnt = 0;

#if FUNC_DUMP
    char *aname = "a.aac";
    char *vname = "v.h264";
    FILE *afile = NULL;             // audio input file
    FILE *vfile = NULL;             // video input file
#endif

    FILE *bfile = NULL;             // bin  output file
    uint8_t *buf = NULL;
    HEADER hd = { 0 };

    // MP4相关变量
    MP4FileHandle hdl = NULL;       // handle
    MP4TrackId vid = 0;             // video track id
    MP4TrackId aid = 0;             // audio track id
    int width = 640;
    int height = 480;
    int framerate = 25;
    int timescale = 90000;
    uint8_t tmp[3];
    uint8_t aac_decoder_conf[2] = { 0 };
    uint8_t *nalu = NULL;
    uint8_t ntype = 0;
    int flag = 1;
    char *oname = "x264.mp4";

    // 打开输入bin文件
    bfile = fopen(iname, "rb");
    MAZASSERT_RETVAL(!bfile, -1, "err: fopen %s", iname);

#if FUNC_DUMP
    // 创建音频和视频文件
    afile = fopen(aname, "wb");
    MAZASSERT_RETVAL(!afile, -1, "err: fopen %s", aname);

    vfile = fopen(vname, "wb");
    MAZASSERT_RETVAL(!vfile, -1, "err: fopen %s", vname);
#endif

    // 申请缓存
    buf = malloc(1024 * 1024);

    // MP4相关
    hdl = MP4Create(oname, 0);
    MAZASSERT_RETVAL(MP4_INVALID_FILE_HANDLE == hdl, -1, "err: MP4Create");
    MP4SetTimeScale(hdl, timescale);

    //aid = MP4AddAudioTrack(hdl, 8000, 768, MP4_MPEG2_AAC_LC_AUDIO_TYPE);
    aid = MP4AddAudioTrack(hdl, 8000, 1024, MP4_MPEG2_AAC_LC_AUDIO_TYPE);
    MAZASSERT_RETVAL(MP4_INVALID_TRACK_ID == aid , -1, "err: MP4AddAudioTrack");
    MP4SetAudioProfileLevel(hdl, 2);

    /*
    首先, config有2个字节组成，共16位，具体含义如下：
    5 bits | 4 bits | 4 bits | 3 bits
    第一个 第二个 第三个 第四个

    // AAC object types
    #define MAIN 1
    #define LOW  2
    #define SSR  3
    #define LTP  4

    第一个：AAC Object Type
    第二个：Sample Rate Index
    第三个：Channel Number
    第四个：Don't care, 设 0
    */

    tmp[0] = 2;
    tmp[1] = 0xB;   // 8Khz
    tmp[2] = 1;
    aac_decoder_conf[0] = ((tmp[0] << 3) & 0xF8) | ((tmp[1] >> 1) & 0x7);
    aac_decoder_conf[1] = ((tmp[1] << 7) & 0x80) | ((tmp[2] << 3) & 0x78);
    MP4SetTrackESConfiguration(hdl, aid, aac_decoder_conf, 2);

    dlog(DINF, "0x%02x/0x%02x", aac_decoder_conf[0], aac_decoder_conf[1]);

    // 循环读取文件
    while(1)
    {
        // 读取码流信息头
        memset(&hd, 0, sizeof(HEADER));
        rcnt = fread(&hd, 1, sizeof(HEADER), bfile);
        MAZASSERT_BRK_NOMSG(rcnt != sizeof(HEADER));

        // 判断码流同步码
        MAZASSERT_BRK(hd.sync != 0x55AA5A5A, "err: sync, 0x%08x", hd.sync);

        // 读取bin文件, 取出一帧码流数据
        rcnt = fread(buf, 1, hd.len, bfile);
        MAZASSERT_BRK(rcnt != hd.len, "err: read %s frame failed", iname);
        scnt++;

        dlog(DIGN, "[%04d] type:%d len:%d", scnt, hd.type, hd.len);

        if(hd.type == MAZ_STREAM_TYPE_AUDIO)
        {
#if FUNC_DUMP
            wcnt = fwrite(buf, 1, hd.len, afile);
            MAZASSERT_BRK(wcnt != hd.len, "err: fwrite %s audio frame header", aname);
#endif
            acnt++;
            MP4WriteSample(hdl, aid, buf + 7, hd.len - 7, 1024, 0, 1);
        }
        else if(hd.type == MAZ_STREAM_TYPE_VIDEO)
        {
#if FUNC_DUMP
            wcnt = fwrite(buf, 1, hd.len, vfile);
            MAZASSERT_BRK(wcnt != hd.len, "err: fwrite %s video frame header", vname);
#endif

            MAZASSERT_BRK_NOMSG(buf[0] != 0 || buf[1] != 0 || buf[2] != 0 || buf[3] != 1);

            vcnt++;
            nalu = buf + 4;
            ntype = nalu[0] & 0x1F;

            switch (ntype)
            {
            case 0x07:  // SPS
                if (flag)
                {
                    vid = MP4AddH264VideoTrack(hdl,
                                               timescale,               // timescale
                                               timescale / framerate,   // timescale / framerate
                                               width,                   // width
                                               height,                  // height
                                               nalu[1],                 // sps[1] AVCProfileIndication
                                               nalu[2],                 // sps[2] profile_compat
                                               nalu[3],                 // sps[3] AVCLevelIndication
                                               3);                      // 4 bytes length before each NAL unit
                    MAZASSERT_RETVAL(MP4_INVALID_TRACK_ID == vid, -1, "err: MP4AddH264VideoTrack");
                    MP4SetVideoProfileLevel(hdl, 0x7F);
                    flag = 0;
                    dlog(DINF, "add the video track");
                    dlog(DINF, "nalu[1] = 0x%02x", nalu[1]);
                    dlog(DINF, "nalu[2] = 0x%02x", nalu[2]);
                    dlog(DINF, "nalu[3] = 0x%02x", nalu[3]);
                }
                dmsg(DIGN, "[%04d] SPS(%d)\n", vcnt, hd.len);
                MP4AddH264SequenceParameterSet(hdl, vid, nalu, hd.len - 4);
                break;
            case 0x08:  // PPS
                dmsg(DIGN, "[%04d] PPS(%d)\n", vcnt, hd.len);
                MP4AddH264PictureParameterSet(hdl, vid, nalu, hd.len - 4);
                break;
            default:
                dmsg(DIGN, "[%04d] FRM(%d)\n", vcnt, hd.len);
                buf[0] = ((hd.len - 4) >> 24) & 0xFF;
                buf[1] = ((hd.len - 4) >> 16) & 0xFF;
                buf[2] = ((hd.len - 4) >> 8) & 0xFF;
                buf[3] = ((hd.len - 4) >> 0) & 0xFF;
                MP4WriteSample(hdl, vid, buf, hd.len, MP4_INVALID_DURATION, 0, 1);
                break;
            }
        }
    }

    // 关闭文件
#if FUNC_DUMP
    fclose(afile);
    fclose(vfile);
#endif
    fclose(bfile);
    MP4Close(hdl, 0);

    return 0;
}

int maz_h264_get_nalu(FILE *ifile, unsigned char *pnalu)
{
    unsigned char c;
    int pos = 0;
    int len;

    if(!ifile)
        return -1;

    if((len = fread(pnalu, 1, 4, ifile)) <= 0)
        return -1;

    if(pnalu[0] != 0 || pnalu[1] != 0 || pnalu[2] != 0 || pnalu[3] != 1)
        return -1;

    pos = 4;

    while(1)
    {
        if(feof(ifile))
            break;
        pnalu[pos] = fgetc(ifile);
        if(pnalu[pos - 3] == 0 && pnalu[pos - 2] == 0 && pnalu[pos - 1] == 0 && pnalu[pos] == 1)
        {
            fseek(ifile, -4, SEEK_CUR);
            pos -= 4;
            break;
        }
        pos++;
    }
    len = pos + 1;
    return len;
}


