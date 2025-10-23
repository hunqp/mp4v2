#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mp4v2/mp4v2.h>
#include "maz_com_assert.h"

int dlvl = DIGN;

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
 *       因此, 1帧音频对应3帧视频, 由于一比三的关系, 只能包含24fps视频, 因此这里最后一帧音频多匹配一帧视频
 */
int maz_split(char *iname)
{
    int i = 0;
    int rcnt = 0;                   // read count
    int wcnt = 0;                   // write count
    int scnt = 0;                   // stream count
    int acnt = 0;
    int vcnt = 0;
    char *aname = "a.aac";
    char *vname = "v.h265";
    FILE *bfile = NULL;            // bin  output file
    FILE *afile = NULL;            // audio input file
    FILE *vfile = NULL;            // video input file
    uint8_t *buf = NULL;
    HEADER hd = { 0 };

    // 打开输入bin文件
    bfile = fopen(iname, "rb");
    MAZASSERT_RETVAL(!bfile, -1, "err: fopen %s", iname);

    // 创建音频和视频文件
    afile = fopen(aname, "wb");
    MAZASSERT_RETVAL(!afile, -1, "err: fopen %s", aname);

    vfile = fopen(vname, "wb");
    MAZASSERT_RETVAL(!vfile, -1, "err: fopen %s", vname);

    // 申请缓存
    buf = malloc(1024 * 1024);

    // 循环读取文件
    while(1)
    {
        // 读取码流信息头
        memset(&hd, 0, sizeof(HEADER));
        rcnt = fread(&hd, 1, sizeof(HEADER), bfile);
        MAZASSERT_BRK(rcnt != sizeof(HEADER), "err: read %s header failed", iname);

        // 判断码流同步码
        MAZASSERT_BRK(hd.sync != 0x55AA5A5A, "err: sync");

        // 读取bin文件, 取出一帧码流数据
        rcnt = fread(buf, 1, hd.len, bfile);
        MAZASSERT_BRK(rcnt != hd.len, "err: read %s frame failed", iname);
        scnt++;

        dlog(DIGN, "[%04d] type:%d len:%d", scnt, hd.type, hd.len);

        if(hd.type == MAZ_STREAM_TYPE_AUDIO)
        {
            wcnt = fwrite(buf, 1, hd.len, afile);
            MAZASSERT_BRK(wcnt != hd.len, "err: fwrite %s audio frame header", aname);
            acnt++;

        }
        else if(hd.type == MAZ_STREAM_TYPE_VIDEO)
        {
            wcnt = fwrite(buf, 1, hd.len, vfile);
            MAZASSERT_BRK(wcnt != hd.len, "err: fwrite %s video frame header", vname);
            vcnt++;
        }
    }

    // 关闭文件
    fclose(afile);
    fclose(vfile);
    fclose(bfile);

    return 0;
}

