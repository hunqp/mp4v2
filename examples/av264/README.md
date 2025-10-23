# 将 H.264 封装为 MP4 文件

## 编译

```
paul@vmware:~/gitee/mp4v2/mp4v2-h265-study/examples/av264$ make
gcc -I./inc -g -c src/main.c -o src/main.o
gcc src/main.o -L./lib -lavformat -lmp4v2 -o av264
strip av264
paul@vmware:~/gitee/mp4v2/mp4v2-h265-study/examples/av264$

```

## 执行

没有指定参数将会打印报错信息，输出用法提示：

```
paul@vmware:~/gitee/mp4v2/mp4v2-h265-study/examples/av264$ ./av264 m264.bin 
=========> delete file 
[0135][maz_split] 0x15/0x88
[0194][maz_split] add the video track
[0195][maz_split] nalu[1] = 0x42
[0196][maz_split] nalu[2] = 0x00
[0197][maz_split] nalu[3] = 0x1e
paul@vmware:~/gitee/mp4v2/mp4v2-h265-study/examples/av264$ 
```

## 成果

在当前目录下输出 a.aac v.264 x264.mp4 文件。
