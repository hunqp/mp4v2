#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mp4v2/mp4v2.h>
#include "maz_com_assert.h"

int dlvl = DINF;

int maz_h265_get_nalu(FILE *ifile, unsigned char *pnalu)
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

int maz_mp4_pack(const char *iname, const char *oname)
{
    int cnt = 0;
    int len = 0;
    int width = 640;
    int height = 480;
    int framerate = 25;
    int timescale = 90000;
    int addStream = 1;
    int s[8] = { 0 };
    unsigned char *nbuf = NULL;         // nalu buffer
    unsigned char *nalu = NULL;
    unsigned char ntype = 0;
    FILE *ifile = NULL;
    MP4FileHandle hdl = NULL;           // handle
    MP4TrackId vid = 0;                 // video track id

    nbuf = malloc(1024 * 1024);
    MAZASSERT_RETVAL(!nbuf, -1, "err: malloc");

    ifile = fopen(iname, "rb");
    MAZASSERT_RETVAL(!ifile, -1, "err: fopen");

    hdl = MP4Create(oname, 0);
    MAZASSERT_RETVAL(MP4_INVALID_FILE_HANDLE == hdl, -1, "err: MP4Create");
    MP4SetTimeScale(hdl, timescale);

    while(1)
    {
        len = maz_h265_get_nalu(ifile, nbuf);
        MAZASSERT_BRK_NOMSG(len <= 0);

        if (nbuf[0] != 0 || nbuf[1] != 0 || nbuf[2] != 0 || nbuf[3] != 1)
        {
            continue;
        }

        len -= 4;
        nalu = nbuf + 4;
        ntype = (nalu[0] & 0x7E) >> 1;

        switch (ntype)
        {
        case 0:     // B
            s[0]++;
            cnt++;

            nbuf[0] = (len >> 24) & 0xFF;
            nbuf[1] = (len >> 16) & 0xFF;
            nbuf[2] = (len >> 8) & 0xFF;
            nbuf[3] = (len >> 0) & 0xFF;
            MP4WriteSample(hdl, vid, nbuf, len + 4, MP4_INVALID_DURATION, 0, 1);

            break;
        case 1:     // P
            s[1]++;
            cnt++;

            nbuf[0] = (len >> 24) & 0xFF;
            nbuf[1] = (len >> 16) & 0xFF;
            nbuf[2] = (len >> 8) & 0xFF;
            nbuf[3] = (len >> 0) & 0xFF;
            MP4WriteSample(hdl, vid, nbuf, len + 4, MP4_INVALID_DURATION, 0, 1);

            break;
        case 19:    // IDR
            s[2]++;
            cnt++;

            nbuf[0] = (len >> 24) & 0xFF;
            nbuf[1] = (len >> 16) & 0xFF;
            nbuf[2] = (len >> 8) & 0xFF;
            nbuf[3] = (len >> 0) & 0xFF;
            MP4WriteSample(hdl, vid, nbuf, len + 4, MP4_INVALID_DURATION, 0, 1);

            break;
        case 32:    // VPS
            s[3]++;
            cnt++;
            dlog(DIGN, "============================");
            dlog(DIGN, "%04d, vps(%d)", cnt, len);

            if(addStream)
            {
                dlog(DBUG, "%d/%d/%d", nalu[1], nalu[2], nalu[3]);
                dlog(DBUG, "%d/%d/%d", nalu[5], nalu[6], nalu[7]);
                dlog(DBUG, "%d/%d/%d", 1, 0, 150);
                vid = MP4AddH265VideoTrack(hdl,
                                           timescale,               // timescale
                                           timescale / framerate,   // timescale / framerate
                                           width,                   // width
                                           height,                  // height
                                           1,                 // AVCProfileIndication
                                           0,                 // profile_compat
                                           150,                 // AVCLevelIndication
                                           3);                      // 4 bytes length before each NAL unit
                MAZASSERT_RETVAL(MP4_INVALID_TRACK_ID == vid, -1, "err: MP4AddH265VideoTrack");
                dlog(DBUG, "PAUL1");
                MP4SetVideoProfileLevel(hdl, 0x7F);
                dlog(DBUG, "PAUL2");
                addStream = 0;
            }

            dlog(DBUG, "PAUL3");
            MP4AddH265VideoParameterSet(hdl, vid, nalu, len);
            dlog(DBUG, "PAUL4");

            break;
        case 33:    // SPS
            s[4]++;
            cnt++;
            dlog(DIGN, "%04d, sps(%d)", cnt, len);

            MP4AddH265SequenceParameterSet(hdl, vid, nalu, len);

            break;
        case 34:    // PPS
            s[5]++;
            cnt++;
            dlog(DIGN, "%04d, pps(%d)", cnt, len);

            MP4AddH265PictureParameterSet(hdl, vid, nalu, len);

            break;
        default:
            s[6]++;
            cnt++;
            dlog(DIGN, "%04d, other(%d)", cnt, len);
            break;
        }
    }

    free(nbuf);
    fclose(ifile);
    MP4Close(hdl, 0);

    dlog(DIGN, "============================");
    dmsg(DINF, "%-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n", "B", "P", "IDR", "VPS", "SPS", "PPS", "DEFAT", "CNT");
    dmsg(DINF, "%-12d %-12d %-12d %-12d %-12d %-12d %-12d %-12d\n", s[0], s[1], s[2], s[3], s[4], s[5], s[6], cnt);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = -1;

    if (argc != 3)
    {
        dlog(DERR, "err: argc = %d", argc);
        dlog(DERR, "usage: %s <iname> <oname>\n", argv[0]);
        return -1;
    }

    ret = maz_mp4_pack(argv[1], argv[2]);
    MAZASSERT_RETVAL(ret, -1, "err: maz_mp4_pack");

    return 0;
}

