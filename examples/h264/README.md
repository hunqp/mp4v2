# 将 H.264 封装为 MP4 文件

## 编译

```
paul@vmware:~/gitee/mp4v2-study/examples/h264$ make
gcc -I./inc -g -c src/main.c -o src/main.o
gcc src/main.o -L./lib -lavformat -lmp4v2 -o h264
strip h264
paul@vmware:~/gitee/mp4v2-study/examples/h264$ 
```

## 执行

没有指定参数将会打印报错信息，输出用法提示：

```
paul@vmware:~/gitee/mp4v2-study/examples/h264$ ./h264 
[0149][main] err: argc = 1
[0150][main] usage: ./h264 <iname> <oname>

paul@vmware:~/gitee/mp4v2-study/examples/h264$
```

正确给出参数后，将进行 MP4 封装动作。

```
paul@vmware:~/gitee/mp4v2-study/examples/h264$ ./h264 test_640x480.h264 out.mp4
0000, sps(20)
0001, pps(4)
0027, sps(20)
0028, pps(4)
0054, sps(20)
0055, pps(4)
0081, sps(20)
0082, pps(4)
0108, sps(20)
0109, pps(4)
0135, sps(20)
0136, pps(4)
0162, sps(20)
0163, pps(4)
0189, sps(20)
0190, pps(4)
0216, sps(20)
0217, pps(4)
0243, sps(20)
0244, pps(4)
0270, sps(20)
0271, pps(4)
0297, sps(20)
0298, pps(4)
0324, sps(20)
0325, pps(4)
0351, sps(20)
0352, pps(4)
0378, sps(20)
0379, pps(4)
s0           s1           s2           s3           sps          pps          frame       
1            0            0            1            15           15           362         
paul@vmware:~/gitee/mp4v2-study/examples/h264$ 
```

## 成果

成果物的文件名依赖于上面执行命令中指定的第二个参数。

```
paul@vmware:~/gitee/mp4v2-study/examples/h264$ ll out.mp4 
-rw-rw-r-- 1 paul paul 11431045 4月  20 22:20 out.mp4
paul@vmware:~/gitee/mp4v2-study/examples/h264$
```

使用常规播放器可以打开播放。我这里使用的是 potplayer。