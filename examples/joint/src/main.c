#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mp4v2/mp4v2.h>
#include "maz_com_assert.h"

int dlvl = DINF;

// AAC 768
// ALAW 1024
//#define MAZ_AUDIO_FRAME_SIZE    768
#define MAZ_AUDIO_FRAME_SIZE    1024

#define MAZ_STREAM_TYPE_AUDIO   0
#define MAZ_STREAM_TYPE_VIDEO   1

typedef struct _HEADER_
{
    int sync;           // 同步码, 值恒为 0x55AA5A5A
    int type;           // 码流类型, 0:音频帧; 1:视频帧
    int num;            // 码流序号
    int len;            // 码流长度
} HEADER;

int maz_h265_get_nalu(FILE *ifile, unsigned char *pnalu)
{
    int rcnt = 0;
    unsigned char c;
    int pos = 0;
    int len;

    MAZASSERT_RETVAL_NOMSG(!ifile, -1);

    rcnt = fread(pnalu, 1, 4, ifile);
    MAZASSERT_RETVAL(rcnt != 4, -1, "err: fread rcnt = %d", rcnt);

    if(pnalu[0] != 0 || pnalu[1] != 0 || pnalu[2] != 0 || pnalu[3] != 1)
    {
        return -1;
    }

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

int maz_joint(char *aname, char *vname, char *oname);

int main(int argc, char *argv[])
{
    int ret = -1;

    if (argc != 4)
    {
        dlog(DERR, "usage: %s <audio> <video> <out.bin>\n", argv[0]);
        return -1;
    }

    ret = maz_joint(argv[1], argv[2], argv[3]);
    MAZASSERT_RETVAL(ret, -1, "err: maz_joint");

    return 0;
}

/**
 * @brief 将 AAC 音频和 H.265 视频拼接为一个文件
 * @note 音频采样率为8k, 一帧音频帧包含1024个采样, 1秒8帧音频
 *       视频帧率为 25fps
 *       因此, 1帧音频对应3帧视频, 由于一比三的关系, 只能包含24fps视频, 因此这里最后一帧音频多匹配一帧视频
 */
int maz_joint(char *aname, char *vname, char *oname)
{

    int i = 0;
    int rcnt = 0;                   // read count
    int wcnt = 0;                   // write count
    int scnt = 0;                   // stream count
    int acnt = 0;
    int vcnt = 0;
    FILE *aifile = NULL;            // audio input file
    FILE *vifile = NULL;            // video input file
    FILE *bofile = NULL;            // bin  output file
    uint8_t *abuf = NULL;
    uint8_t *vbuf = NULL;
    HEADER ahd = { 0 };
    HEADER vhd = { 0 };

    // 打开音频和视频文件
    aifile = fopen(aname, "rb");
    MAZASSERT_RETVAL(!aifile, -1, "err: fopen %s", aname);

    vifile = fopen(vname, "rb");
    MAZASSERT_RETVAL(!vifile, -1, "err: fopen %s", vname);

    // 申请缓存, AAC 一帧大小恒定为768字节
    abuf = malloc(MAZ_AUDIO_FRAME_SIZE);
    vbuf = malloc(1024 * 1024);

    // 创建输出bin文件
    bofile = fopen(oname, "wb");
    MAZASSERT_RETVAL(!bofile, -1, "err: fopen %s", oname);

    // 循环读取音频和视频文件, 并写入bin文件中
    // 强制要求, 音频的时长比视频时长要长, 以视频结束为退出循环的依据
    while(1)
    {
        // 读取一帧音频帧
        rcnt = fread(abuf, 1, MAZ_AUDIO_FRAME_SIZE, aifile);
        MAZASSERT_BRK(rcnt != MAZ_AUDIO_FRAME_SIZE, "err: read %s", aname);

        // 构建音频码流信息头
        memset(&ahd, 0, sizeof(HEADER));
        ahd.sync = 0x55AA5A5A;
        ahd.type = MAZ_STREAM_TYPE_AUDIO;
        ahd.num = scnt++;
        ahd.len = MAZ_AUDIO_FRAME_SIZE;
        acnt++;

        // 写入音频帧到bin文件
        wcnt = fwrite(&ahd, 1, sizeof(HEADER), bofile);
        MAZASSERT_BRK(wcnt != sizeof(HEADER), "err: fwrite %s audio frame header", oname);

        wcnt = fwrite(abuf, 1, ahd.len, bofile);
        MAZASSERT_BRK(wcnt != ahd.len, "err: fwrite %s audio frame header", oname);
        dlog(DINF, "scnt = %d, ----------------- audio %d -----------------", scnt, acnt);

        // 读取三帧视频帧
        //===============================================================================
        memset(&vhd, 0, sizeof(HEADER));
        vhd.len = maz_h265_get_nalu(vifile, vbuf);
        MAZASSERT_BRK(vhd.len <= 0, "err: maz_h265_get_nalu, ret = %d", vhd.len);

        // 构建视频码流信息头
        vhd.sync = 0x55AA5A5A;
        vhd.type = MAZ_STREAM_TYPE_VIDEO;
        vhd.num = ++scnt;
        vcnt++;

        // 写入视频帧到bin文件
        wcnt = fwrite(&vhd, 1, sizeof(HEADER), bofile);
        MAZASSERT_BRK(wcnt != sizeof(HEADER), "err: fwrite %s video frame header", oname);

        wcnt = fwrite(vbuf, 1, vhd.len, bofile);
        MAZASSERT_BRK(wcnt != vhd.len, "err: fwrite %s video frame header", oname);
        dlog(DINF, "scnt = %d", scnt);

        //===============================================================================
        memset(&vhd, 0, sizeof(HEADER));
        vhd.len = maz_h265_get_nalu(vifile, vbuf);
        MAZASSERT_BRK(vhd.len <= 0, "err: maz_h265_get_nalu, ret = %d", vhd.len);

        // 构建视频码流信息头
        vhd.sync = 0x55AA5A5A;
        vhd.type = MAZ_STREAM_TYPE_VIDEO;
        vhd.num = ++scnt;
        vcnt++;

        // 写入视频帧到bin文件
        wcnt = fwrite(&vhd, 1, sizeof(HEADER), bofile);
        MAZASSERT_BRK(wcnt != sizeof(HEADER), "err: fwrite %s video frame header", oname);

        wcnt = fwrite(vbuf, 1, vhd.len, bofile);
        MAZASSERT_BRK(wcnt != vhd.len, "err: fwrite %s video frame header", oname);
        dlog(DINF, "scnt = %d", scnt);

        //===============================================================================
        memset(&vhd, 0, sizeof(HEADER));
        vhd.len = maz_h265_get_nalu(vifile, vbuf);
        MAZASSERT_BRK(vhd.len <= 0, "err: maz_h265_get_nalu, ret = %d", vhd.len);

        // 构建视频码流信息头
        vhd.sync = 0x55AA5A5A;
        vhd.type = MAZ_STREAM_TYPE_VIDEO;
        vhd.num = ++scnt;
        vcnt++;

        // 写入视频帧到bin文件
        wcnt = fwrite(&vhd, 1, sizeof(HEADER), bofile);
        MAZASSERT_BRK(wcnt != sizeof(HEADER), "err: fwrite %s video frame header", oname);

        wcnt = fwrite(vbuf, 1, vhd.len, bofile);
        MAZASSERT_BRK(wcnt != vhd.len, "err: fwrite %s video frame header", oname);
        dlog(DINF, "scnt = %d", scnt);

        //===============================================================================
        // 判断音频帧是否是第8/16/24...帧, 是的话, 多读取一帧
        if((acnt % 8) == 0)
        {
            memset(&vhd, 0, sizeof(HEADER));
            vhd.len = maz_h265_get_nalu(vifile, vbuf);
            MAZASSERT_BRK(vhd.len <= 0, "err: maz_h265_get_nalu, ret = %d", vhd.len);

            // 构建视频码流信息头
            vhd.sync = 0x55AA5A5A;
            vhd.type = MAZ_STREAM_TYPE_VIDEO;
            vhd.num = ++scnt;
            vcnt++;

            // 写入视频帧到bin文件
            wcnt = fwrite(&vhd, 1, sizeof(HEADER), bofile);
            MAZASSERT_BRK(wcnt != sizeof(HEADER), "err: fwrite %s video frame header", oname);

            wcnt = fwrite(vbuf, 1, vhd.len, bofile);
            MAZASSERT_BRK(wcnt != vhd.len, "err: fwrite %s video frame header", oname);
            dlog(DINF, "scnt = %d --- more ---", scnt);
        }
    }

    // 关闭文件
    fclose(aifile);
    fclose(vifile);
    fclose(bofile);

    return 0;
}

