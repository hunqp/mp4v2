#include <iostream>
#include <fstream>
#include <unistd.h>
#include "mp4v2_mp4.h"

const char* videoFileName  = "video.h264";
const char* audioFileName  = "audio.aac";
const char* outputFileName = "testMP4.mp4";

int main() {

    // Đọc dữ liệu từ file video
    std::ifstream videoFile(videoFileName, std::ios::binary | std::ios::ate);
    if (!videoFile.is_open()) {
        std::cerr << "Error opening video file: " << videoFileName << std::endl;
        return 1;
    }
    std::streampos videoSize = videoFile.tellg();
    videoFile.seekg(0, std::ios::beg);
    unsigned char *videoBuffer = (unsigned char *)malloc(videoSize);
    if (videoBuffer == NULL) {
        std::cerr << "Malloc videoSize: " << videoSize << std::endl;
        return 1;
    }
    videoFile.read(reinterpret_cast<char*>(videoBuffer), videoSize);
    videoFile.close();


    // Đọc dữ liệu từ file audio
    std::ifstream audioFile(audioFileName, std::ios::binary | std::ios::ate);
    if (!audioFile.is_open()) {
        std::cerr << "Error opening audio file: " << audioFileName << std::endl;
        return 1;
    }
    std::streampos audioSize = audioFile.tellg();
    audioFile.seekg(0, std::ios::beg);
    unsigned char *audioBuffer = (unsigned char *)malloc(audioSize);
    if (audioBuffer == NULL) {
        std::cerr << "Malloc audioSize: " << audioSize << std::endl;
        return 1;
    }
    audioFile.read(reinterpret_cast<char*>(audioBuffer), audioSize);
    audioFile.close();

    // Tạo file MP4 và kiểm tra lỗi
    // MP4V2_MP4_T *mp4Handle = Mp4v2CreateMP4File(outputFileName, 1280, 720, 8000, 13);
    MP4V2_MP4_T *mp4Handle = Mp4v2CreateMP4File(outputFileName, 1280, 720, 16000, 13);
    // MP4V2_MP4_T *mp4Handle = Mp4v2CreateMP4File(outputFileName, 1920, 1080, 90000, 30);
    if (!mp4Handle) {
        std::cerr << "Error creating MP4 file" << std::endl;
        return 1;
    }

    int offsetVideo = 0;
    int offsetAudio = 0;
    int remainLen = audioSize;
    int readLen;
    while (1) {

        MP4ENC_NaluUnit nalueRead;
        int read = ReadOneNaluFromBuf(videoBuffer, videoSize, offsetVideo, nalueRead);
        if (read) {
            Mp4v2WriteH264toMP4(mp4Handle, &videoBuffer[offsetVideo], read);
            offsetVideo += read;
        }
        else {
            break;
        }
        
        if (remainLen >= 1024) {
            readLen = 1024;
        }
        else {
            readLen = remainLen;
        }
        Mp4v2WriteAACtoMP4(mp4Handle, &audioBuffer[offsetAudio], readLen);
        // std::cout << "Mp4v2WriteAACtoMP4 readLen:" << readLen << std::endl;
        remainLen -= readLen;
        offsetAudio += readLen;
        if (remainLen == 0) {
            // break;
            remainLen = audioSize;
            offsetAudio = 0;
        }

        // Fake frame generator
        usleep(13000);
    }

    // Đóng file MP4
    Mp4v2CloseMP4File(mp4Handle);

    std::cout << "MP4 file created successfully: " << outputFileName << std::endl;

    return 0;
}
